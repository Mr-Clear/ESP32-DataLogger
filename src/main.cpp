#include "AdcFiber.h"
#include "Ds18b20Fiber.h"
#include "Gpio.h"
#include "HttpPostTask.h"
#include "Sht30Fiber.h"
#include "SystemDataFiber.h"
#include "Tft.h"
#include "WifiKeepAliveTask.h"

#include <FiberQueueTask.h>
#include <JetBrainsMono15.h>
#include <Observable.h>

#include <DallasTemperature.h>
#include <esp_sntp.h>
#include <OneWire.h>
#include <freertos/semphr.h>

#include <mutex>
#include <optional>
#include <vector>

#define ADC_PIN 34
#define BUTTON_1 0
#define BUTTON_2 35

constexpr const Vector2i displayTiling{120, 16};
#define POS(x, y) displayTiling * Vector2i(x, y)

std::optional<time_t> getTime();
String formatTime(const std::optional<time_t> &timestamp);

int numberOfDevices;
DeviceAddress tempDeviceAddress;

const char* ntpServer = "fritz.box";
const char* time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";  // TimeZone rule for Europe/Rome including daylight adjustment rules (optional)

using Callback = std::function<void()>;

Tft tft(32);
WifiKeepAliveTask wifiTask;

FiberQueueTask fiberQueueTask1(1000, "FiberQueueTask1", 8192, 10, Task::Core::Core1);
AdcFiber adcFiber;
Sht30Fiber sht30Fiber;
Ds18b20Fiber ds18b20Fiber(17);
SystemDataFiber systemDataFiber;

HttpPostTask httpPostTask(std::bind(&WifiKeepAliveTask::isWifiConnected, &wifiTask));

using ValueTask = QueueTask<Callback>;
std::vector<ValueTask*> valueTasks{&fiberQueueTask1};
SemaphoreHandle_t valuesSemaphore;

PostData postData;
std::mutex postDataMutex;

void setup(void) {
  Serial.begin(115200);
  Serial.println();

  Serial.println("Connect WIFI...");
  sntp_servermode_dhcp(0);
  configTzTime(time_zone, ntpServer);

  fiberQueueTask1.addFiber(adcFiber);
  fiberQueueTask1.addFiber(sht30Fiber);
  fiberQueueTask1.addFiber(ds18b20Fiber);
  fiberQueueTask1.addFiber(systemDataFiber);

  tft.start();
  wifiTask.start();
  httpPostTask.start();

  for (ValueTask *valueTask : valueTasks) {
    valueTask->start();
  }
  Serial.println(valueTasks.size());
  valuesSemaphore = xSemaphoreCreateCounting(valueTasks.size(), 0);

  tft.loadFont(JetBrainsMono15);
  tft.setRotation(3);
  tft.fillScreen({4, 4, 4});

  adcFiber.channel(ADC_PIN).addObserver([] (double v) {
    const double voltage = v * 2.0;
    std::lock_guard<std::mutex> lck(postDataMutex);
    postData.voltage = voltage;
    const String voltageString = "VCC: " + String(voltage) + " V  ";
    tft.drawString(voltageString, POS(0, 4));
  });

  sht30Fiber.data().addObserver( [] (const Sht30Fiber::Data &data) {
    std::lock_guard<std::mutex> lck(postDataMutex);
    postData.sht30Temperature = data.error ? NAN : data.temperature;
    postData.sht30Humidity = data.error ? NAN : data.humidity;

    const String tmp = (data.error ? ("Tmp Err: " + String(data.error)) : ("Tmp: " + String(data.temperature))) + "       ";
    const String hum = (data.error ? ("Hum Err: " + String(data.error)) : ("Hum: " + String(data.humidity))) + "       ";
    tft.setTextColor(Color::White, Color::Black);
    tft.drawString(tmp, POS(0, 5));
    tft.drawString(hum, POS(0, 6));
  });

  ds18b20Fiber.data().addObserver( [] ( const std::map<String, float> &values) {
    std::vector<double> ds18b20;
    std::vector<String> ds18b20Strings;
    int row = 0;
    for (const auto & [address, temperature] : values) {
      ds18b20.emplace_back(temperature);
      tft.setTextColor(Color::White, Color::Black);
      tft.drawString("Tmp: " + String(temperature) + " °C", POS(1, row));
      ++row;
    }
    std::lock_guard<std::mutex> lck(postDataMutex);
    postData.ds18b20 = ds18b20;
  });

  systemDataFiber.data().addObserver( [] (const SystemDataFiber::Data &values) {
    tft.setTextColor(Color::White, Color::Black);
    const String memoryTotal = "Mem: " + String(ESP.getHeapSize()) + "       ";
    const String memoryFree = "Fre: " + String(ESP.getFreeHeap()) + "       ";
    tft.drawString(memoryTotal, POS(1, 4));
    tft.drawString(memoryFree, POS(1, 5));
  });

  wifiTask.localIp().addObserver( [] (const IPAddress &ip) {
    tft.setTextColor(Color::White, Color::Black);
    const String ips = ip.toString() + "      ";
    tft.drawString(ips, POS(1, 6));
  }, true);

  wifiTask.wifiStatusText().addObserver( [] (const String &status) {
    tft.setTextColor(Color::White, Color::Black);
    const String wifiStatusString = status + "           ";
    tft.drawString(wifiStatusString, POS(0, 7));
  }, true);
  
  addGpioEvent(BUTTON_2, PinInputMode::PullUp, [] (uint8_t, GpioEventType type) {
    if (type == GpioEventType::Falling) {
      const int blVal = tft.getBackLite() / 2;
      tft.setTextColor(Color::White, Color::Black);
      tft.drawString(String(tft.setBackLite(blVal)) + "       ", POS(1, 3));
    }
  });

  addGpioEvent(BUTTON_1, PinInputMode::PullUp, [] (uint8_t, GpioEventType type) {
    if (type == GpioEventType::Falling) {
      const int blVal = max(int(tft.getBackLite()) * 2, 1);
      tft.setTextColor(Color::White, Color::Black);
      tft.drawString(String(tft.setBackLite(blVal)) + "       ", POS(1, 3));
    }
  });

  tft.drawString(String(tft.getBackLite()) + "       ", POS(1, 3));
}

unsigned long lastDuration = 0;
unsigned long lastDelay = 0;
unsigned long lastStart = 0;
void loop() {
  const unsigned long start = millis();
  const unsigned long loopTime = start - lastStart;
  lastStart = start;
  
  for (ValueTask *valueTask : valueTasks) {
    valueTask->send([]{ xSemaphoreGive(valuesSemaphore); });
  }
  
  for (int i = 0; i < valueTasks.size(); i++) {
    xSemaphoreTake(valuesSemaphore, portMAX_DELAY);
  }

  httpPostTask.send(postData);

  const String interval = "Itr: " + String(loopTime) + " ms  ";
  const String fps = "FPS: " + String(1000.0 / loopTime);
  const String lastDurationString = "Dur: " + String(lastDuration) + " ms  ";

  const std::optional<time_t> timestamp = getTime();
  const String time = formatTime(timestamp) + "               ";

  tft.setRotation(3);
  tft.setTextColor(Color::White, Color::Black);
  tft.drawString("Hallo", POS(0, 0));

  tft.drawString(time, POS(0, 0));
  tft.drawString(fps, POS(0, 1));
  tft.drawString(interval, POS(0, 2));
  tft.drawString(lastDurationString, POS(0, 3));

  const int desiredInterval = 1000;
  lastDuration = millis() - start;
  lastDelay = constrain(desiredInterval - lastDuration, 0, desiredInterval);
  delay(lastDelay);
}

std::optional<time_t> getTime() {
  time_t now;
  time(&now);
  if (now > 946684800) // 2000-01-01T00:00:00
    return now;
  return {};
}

String formatTime(const std::optional<time_t> &timestamp) {
  if (timestamp) {
    tm timeinfo;
    localtime_r(&timestamp.value(), &timeinfo);
    char buf[64];
    size_t written = strftime(buf, 64, "%a, %Y-%m-%d %H:%M:%S", &timeinfo);
    return String(buf, written);
  }
  return "NULL";
}

#include "AdcFiber.h"
#include "Bme280Fiber.h"
#include "Ds18b20Fiber.h"
#include "Gpio.h"
#include "HttpPostTask.h"
#include "WifiKeepAliveTask.h"

#include <FiberQueueTask.h>
#include <JetBrainsMono15.h>
#include <Observable.h>

#include <DallasTemperature.h>
#include <esp_sntp.h>
#include <OneWire.h>
#include <freertos/semphr.h>

#include <cmath>
#include <mutex>
#include <optional>
#include <vector>

#define BUTTON_PIN 0
#define LED_PIN 2

/*
rrd.create('dataOffice',
            [ds('Duration', heartbeat=5),
            ds('Temp', heartbeat=5, min=-30, max=70),
            ds('Hum', heartbeat=5, min=0, max=100),
            ds('Press', heartbeat=5, min=0, max=100),
            ds('TempIntern', heartbeat=5, min=-30, max=70),
            ds('TempDoor', heartbeat=5, min=-30, max=70),
            ds('TempWindow', heartbeat=5, min=-30, max=70),
            ds('TempRadiator', heartbeat=5, min=-30, max=70)],
            [r(t.AVERAGE, 0.9, timedelta(seconds=1), timedelta(days=31)),
            r(t.AVERAGE, 0.5, timedelta(seconds=10), timedelta(days=366)),
            r(t.AVERAGE, 0.5, timedelta(hours=1), timedelta(days=3653)),
            r(t.MIN, 0.5, timedelta(hours=1), timedelta(days=3653)),
            r(t.MAX, 0.5, timedelta(days=1), timedelta(days=36525)),
            r(t.AVERAGE, 0.5, timedelta(days=1), timedelta(days=36525)),
            r(t.MIN, 0.5, timedelta(days=1), timedelta(days=36525)),
            r(t.MAX, 0.5, timedelta(days=1), timedelta(days=36525))],
            step=timedelta(seconds=1), overwrite=True)
*/

std::optional<time_t> getTime();
String formatTime(const std::optional<time_t> &timestamp);

int numberOfDevices;
DeviceAddress tempDeviceAddress;

const char* ntpServer = "fritz.box";
const char* time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";  // TimeZone rule for Europe/Rome including daylight adjustment rules (optional)

using Callback = std::function<void()>;

PostData postData;
std::mutex postDataMutex;

WifiKeepAliveTask wifiTask;

Bme280Fiber bme280Fiber;
FiberQueueTask fiberQueueTask1(1000, "FiberQueueTask1", 8192, 10, Task::Core::Core1);
std::vector<std::tuple<int, Ds18b20Fiber*>> ds18b20Ports
  {{{13, nullptr},
    {16, nullptr},
    {17, nullptr},
    {18, nullptr}}};

HttpPostTask httpPostTask(std::bind(&WifiKeepAliveTask::isWifiConnected, &wifiTask));

using ValueTask = QueueTask<Callback>;
std::vector<ValueTask*> valueTasks{&fiberQueueTask1};
SemaphoreHandle_t valuesSemaphore;

void setup(void) {
  Serial.begin(115200);
  sleep(1);
  Serial.println("STARTING");
  sntp_servermode_dhcp(0);
  configTzTime(time_zone, ntpServer);

  fiberQueueTask1.addFiber(bme280Fiber);
  for(auto &[port, fiber] : ds18b20Ports) {
    fiber = new Ds18b20Fiber(port); // live long and prosper
    fiberQueueTask1.addFiber(*fiber);
    fiber->data().addObserver( [port] ( const std::map<String, float> &values) {
      for (const auto &[address, value] : values) {
        //Serial.println(String("DS18B20 ") + port + ": " + address + " -> " + value);
      }
      std::lock_guard<std::mutex> lck(postDataMutex);
      postData.ds18b20.insert(values.begin(), values.end());
    });
  }

  bme280Fiber.data().addObserver( [] (const Bme280Fiber::Data &data) {
    //Serial.println(String("BME280: ") + data.temperature + "°C, " + data.humidity + "%, " + data.pressureSeaLevel + "Pa");
    std::lock_guard<std::mutex> lck(postDataMutex);
    postData.bme280Temperature = data.temperature;
    postData.bme280Humidity = data.humidity;
    postData.bme280SeaLevelPressure = data.pressureSeaLevel;
  });

  wifiTask.start();
  httpPostTask.start();

  for (ValueTask *valueTask : valueTasks) {
    valueTask->start();
  }
  valuesSemaphore = xSemaphoreCreateCounting(valueTasks.size(), 0);

  addGpioEvent(BUTTON_PIN, PinInputMode::PullUp, [] (uint8_t, GpioEventType type) {
    if(type == GpioEventType::Falling) {
      Serial.println("Rescan...");
      for(auto &[_, fiber] : ds18b20Ports) {
        fiber->scan();
      }
      Serial.println("Rescan done.");
    }
  });

  wifiTask.wifiStatusText().addObserver( [] (const String &status) {
      Serial.print("WIFI: ");
      Serial.println(status);
  });

  wifiTask.localIp().addObserver( [] (const IPAddress &address) {
      Serial.print("IP address: ");
      Serial.println(address.toString());
  });

  pinMode(LED_PIN, OUTPUT);
}

unsigned long lastDuration = 0;
unsigned long lastDelay = 0;
unsigned long lastStart = 0;
time_t lastSend = 0;
bool ledOn = true;
void loop() {
  const unsigned long start = millis();
  const unsigned long loopTime = start - lastStart;
  lastStart = start;

  std::optional<time_t> currentTime = getTime();
  {
    std::lock_guard<std::mutex> lck(postDataMutex);
    postData = {};
    postData.timestamp = currentTime;
  }
  
  for(ValueTask *valueTask : valueTasks) {
    valueTask->send([]{ xSemaphoreGive(valuesSemaphore); });
  }
  
  for(int i = 0; i < valueTasks.size(); i++) {
    xSemaphoreTake(valuesSemaphore, portMAX_DELAY);
  }

  const bool sendQueueEmpty = httpPostTask.queue().empty();
  if(sendQueueEmpty || currentTime.has_value()) {
    PostData postDataCopy;
    {
      std::lock_guard<std::mutex> lck(postDataMutex);
      postData.duration = millis() - start;
      postDataCopy = postData;
    }
    bool send = sendQueueEmpty;
    if(!sendQueueEmpty) {
      const double expFactor = 0.026896345;  // With this value, the buffer runs full in 7 days.
      const int sendInterval = static_cast<int>(std::pow(2, expFactor * httpPostTask.queue().size()));
      send = currentTime.has_value() ? (difftime(currentTime.value(), lastSend) >= sendInterval) : true;
    }
    if(send) {
      httpPostTask.send(postDataCopy, 100);
      lastSend = currentTime.value_or(0);
    }
  }

  digitalWrite(LED_PIN, (ledOn && wifiTask.isWifiConnected()) ? HIGH : LOW);
  ledOn = !ledOn;

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

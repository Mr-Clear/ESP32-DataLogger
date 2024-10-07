#include "AdcFiber.h"
#include "Ds18b20Fiber.h"
#include "Gpio.h"
#include "HttpPostTask.h"
#include "Sht30Fiber.h"
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
rrd.create('dataBalkon',
            [ds('Duration', heartbeat=5),
            ds('Voltage', heartbeat=5),
            ds('TempAussen', heartbeat=5),
            ds('HumAussen', heartbeat=5),
            ds('TempIntern', heartbeat=5),
            ds('TempLinks', heartbeat=5),
            ds('TempRechts', heartbeat=5)],
            [r(t.AVERAGE, 0.9, timedelta(seconds=1), timedelta(days=31)),
            r(t.AVERAGE, 0.5, timedelta(minutes=1), timedelta(days=366)),
            r(t.MIN, 0.5, timedelta(minutes=1), timedelta(days=366)),
            r(t.MAX, 0.5, timedelta(minutes=1), timedelta(days=366)),
            r(t.AVERAGE, 0.5, timedelta(hours=1), timedelta(days=3653)),
            r(t.MIN, 0.5, timedelta(hours=1), timedelta(days=3653)),
            r(t.MAX, 0.5, timedelta(days=1), timedelta(days=36525)),
            r(t.AVERAGE, 0.5, timedelta(days=1), timedelta(days=36525)),
            r(t.MIN, 0.5, timedelta(days=1), timedelta(days=36525)),
            r(t.MAX, 0.5, timedelta(days=1), timedelta(days=36525))],
            step=timedelta(seconds=1), overwrite=True)

rrdtool create dataBalkon.rrd --step 1 --start 1672527600 DS:Duration:GAUGE:5:U:U DS:Voltage:GAUGE:5:U:U DS:TempAussen:GAUGE:5:U:U DS:HumAussen:GAUGE:5:U:U DS:TempIntern:GAUGE:5:U:U DS:TempLinks:GAUGE:5:U:U DS:TempRechts:GAUGE:5:U:U RRA:AVERAGE:0.9:1:2678400 RRA:AVERAGE:0.5:60:527040 RRA:MIN:0.5:60:527040 RRA:MAX:0.5:60:527040 RRA:AVERAGE:0.5:3600:87672 RRA:MIN:0.5:3600:87672 RRA:MAX:0.5:86400:36525 RRA:AVERAGE:0.5:86400:36525 RRA:MIN:0.5:86400:36525 RRA:MAX:0.5:86400:36525
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

FiberQueueTask fiberQueueTask1(1000, "FiberQueueTask1", 8192, 10, Task::Core::Core1);
Sht30Fiber sht30Fiber;
std::array<std::tuple<int, float&>, 2> ds18b20Ports{{{13, postData.ds18b20_13}, {16, postData.ds18b20_16}}};

HttpPostTask httpPostTask(std::bind(&WifiKeepAliveTask::isWifiConnected, &wifiTask));

using ValueTask = QueueTask<Callback>;
std::vector<ValueTask*> valueTasks{&fiberQueueTask1};
SemaphoreHandle_t valuesSemaphore;


void setup(void) {
  Serial.begin(115200);
  Serial.println();

  Serial.println("Connect WIFI...");
  sntp_servermode_dhcp(0);
  configTzTime(time_zone, ntpServer);

  fiberQueueTask1.addFiber(sht30Fiber);

  for(auto [port, value] : ds18b20Ports) {
    auto f = new Ds18b20Fiber(port); // live long and prosper
    fiberQueueTask1.addFiber(*f);
    f->data().addObserver( [&value] ( const std::map<String, float> &values) {
      std::lock_guard<std::mutex> lck(postDataMutex);
      const auto i = values.begin();
      if(i != values.end()) {
        value = i->second;
      } else {
        value = NAN;
      }
    });
  }

  wifiTask.start();
  httpPostTask.start();

  for (ValueTask *valueTask : valueTasks) {
    valueTask->start();
  }
  valuesSemaphore = xSemaphoreCreateCounting(valueTasks.size(), 0);

  sht30Fiber.data().addObserver( [] (const Sht30Fiber::Data &data) {
    std::lock_guard<std::mutex> lck(postDataMutex);
    postData.sht30Temperature = data.error ? NAN : data.temperature;
    postData.sht30Humidity = data.error ? NAN : data.humidity;

    const String tmp = (data.error ? ("Tmp Err: " + String(data.error)) : ("Tmp: " + String(data.temperature)));
    const String hum = (data.error ? ("Hum Err: " + String(data.error)) : ("Hum: " + String(data.humidity)));

    Serial.println(tmp + ", " + hum);
  });

  addGpioEvent(BUTTON_PIN, PinInputMode::PullUp, [] (uint8_t, GpioEventType type) {
    if(type == GpioEventType::Falling) {
      sht30Fiber.scan();
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

  std::optional<time_t> time = getTime();
  {
    std::lock_guard<std::mutex> lck(postDataMutex);
    postData = {};
    postData.timestamp = time;
  }
  
  for(ValueTask *valueTask : valueTasks) {
    valueTask->send([]{ xSemaphoreGive(valuesSemaphore); });
  }
  
  for(int i = 0; i < valueTasks.size(); i++) {
    xSemaphoreTake(valuesSemaphore, portMAX_DELAY);
  }

  const bool sendQueueEmpty = httpPostTask.queue().empty();
  if(sendQueueEmpty || time.has_value()) {
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
      send = time.has_value() ? (difftime(time.value(), lastSend) >= sendInterval) : true;
    }
    if(send) {
      httpPostTask.send(postDataCopy, 100);
      lastSend = time.value_or(0);
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

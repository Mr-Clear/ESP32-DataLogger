#include "HttpPostTask.h"
#include "SensorData.h"
#include "Sht30Fiber.h"
#include "Tft.h"
#include "WifiKeepAliveTask.h"

#include <FiberTask.h>
#include <JetBrainsMono15.h>

#include <DallasTemperature.h>
#include <esp_adc_cal.h>
#include <esp_sntp.h>
#include <OneWire.h>

#include <vector>

#define ADC_PIN 34
#define ONE_WIRE_BUS 17
#define BUTTON_1 0
#define BUTTON_2 35

void printAddress(DeviceAddress deviceAddress);
String getTime();
String postData();

int vref = 1100;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
int numberOfDevices;
DeviceAddress tempDeviceAddress;

const char* ntpServer = "fritz.box";
const char* time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";  // TimeZone rule for Europe/Rome including daylight adjustment rules (optional)

SensorData sensorData;

Tft tft(32);
WifiKeepAliveTask wifiTask;
FiberTask fiberTask1(1000, "FiberTask0", 8192, 10, Task::Core::Core1);
Sht30Fiber sht30Fiber(sensorData);
HttpPostTask httpPostTask(1000, std::bind(&WifiKeepAliveTask::isWifiConnected, &wifiTask), postData);

void setup(void) {
  Serial.begin(115200);
  Serial.println();

  tft.loadFont(JetBrainsMono15);
  tft.setRotation(3);
  tft.fillScreen(Color::Black);

  Serial.println("Init ADC...");
  esp_adc_cal_characteristics_t adc_chars;
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);    //Check type of calibration value used to characterize ADC
  if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
      Serial.printf("eFuse Vref:%u mV", adc_chars.vref);
      vref = adc_chars.vref;
  } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
      Serial.printf("Two Point --> coeff_a:%umV coeff_b:%umV\n", adc_chars.coeff_a, adc_chars.coeff_b);
  } else {
      Serial.println("Default Vref: 1100mV");
  }

  Serial.println("Init Temp Sensors...");
  sensors.begin();
  numberOfDevices = sensors.getDeviceCount();
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(numberOfDevices, DEC);
  Serial.println(" devices.");
  for(int i=0;i<numberOfDevices; i++){
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i)) {
      Serial.print("Found device ");
      Serial.print(i, DEC);
      Serial.print(" with address: ");
      printAddress(tempDeviceAddress);
      Serial.println();
    } else {
      Serial.print("Found ghost device at ");
      Serial.print(i, DEC);
      Serial.print(" but could not detect address. Check power and cabling");
      Serial.println();
    }
  }


  Serial.println("Init Buttons...");
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);

  Serial.println("Connect WIFI...");
  sntp_servermode_dhcp(0);
  configTzTime(time_zone, ntpServer);

  Serial.println("Initialization completed.");

  fiberTask1.addFiber(sht30Fiber);

  tft.start();
  wifiTask.start();
  fiberTask1.start();
  httpPostTask.start();
}

unsigned long lastDuration = 0;
unsigned long lastDelay = 0;
unsigned long lastStart = 0;
void loop() {
  const unsigned long start = millis();
  const unsigned long loopTime = start - lastStart;
  lastStart = start;
  const String interval = "Itr: " + String(loopTime) + " ms  ";
  const String fps = "FPS: " + String(1000.0 / loopTime);
  const String lastDurationString = "Dur: " + String(lastDuration) + " ms  ";

  const String time = getTime() + "               ";
  //String time = String(esp_log_timestamp() / 1000);

  const uint16_t v = analogRead(ADC_PIN);
  sensorData.voltage = v / 4095.0 * 2.0 * 3.3 * (vref / 1000.0);
  const String voltageString = "VCC: " + String(sensorData.voltage) + " V  ";

  const String wifiStatusString = wifiTask.wifiStatusText() + "           ";

  std::vector<double> ds18b20;
  std::vector<String> ds18b20Strings;
  sensors.requestTemperatures();
  for (int i=0; i < numberOfDevices; i++) {
    if (sensors.getAddress(tempDeviceAddress, i)) {
      float tempC = sensors.getTempC(tempDeviceAddress);
      ds18b20.emplace_back(tempC);
      ds18b20Strings.emplace_back("Tm:" + String(i) + " " + String(tempC) + " °C");
    }
  }
  sensorData.ds18b20 = ds18b20;

  const String button1 = "BT1: " + String(digitalRead(BUTTON_1) ? "Up     " : "Down ");
  const String button2 = "BT2: " + String(digitalRead(BUTTON_2) ? "Up     " : "Down ");

  const String sht30TemperatureString = (sensorData.sht30Error ? ("Tmp Err: " + String(sensorData.sht30Error)) : ("Tmp: " + String(sensorData.sht30Temperature))) + "       ";
  const String sht30HumidityString = (sensorData.sht30Error ? ("Hum Err: " + String(sensorData.sht30Error)) : ("Hum: " + String(sensorData.sht30Humidity))) + "       ";
  const String ip = wifiTask.localIp() + "      ";

  const String memoryTotal = "Mem: " + String(ESP.getHeapSize()) + "       ";
  const String memoryFree = "Fre: " + String(ESP.getFreeHeap()) + "       ";

  tft.setRotation(3);
  tft.setTextColor(Color::White, Color::Black);
  //tft.fillScreen(Color::Black);

  const Vector2i shift{0, 16};
  Vector2i pos = -shift;

  tft.drawString(time, pos+=shift);
  tft.drawString(fps, pos+=shift);
  tft.drawString(interval, pos+=shift);
  tft.drawString(lastDurationString, pos+=shift);
  tft.drawString(voltageString, pos+=shift);
  tft.drawString(sht30TemperatureString, pos+=shift);
  tft.drawString(sht30HumidityString, pos+=shift);
  tft.drawString(wifiStatusString, pos+=shift);

  pos = Vector2i{tft.size().x() / 2, 0} - shift;
  tft.drawString("", pos+=shift);
  for (const String &s : ds18b20Strings) {
    tft.drawString(s, pos+=shift);
  }
  //tft.drawString(button1, pos+=shift);
  //tft.drawString(button2, pos+=shift);
  tft.drawString(memoryTotal, pos+=shift);
  tft.drawString(memoryFree, pos+=shift);
  tft.drawString(ip, pos+=shift);

  const int desiredInterval = 1000;
  lastDuration = millis() - start;
  lastDelay = constrain(desiredInterval - lastDuration, 0, desiredInterval);
  delay(lastDelay);
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++){
    if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
  }
}

String getTime() {
  return String(millis() / 1000);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return String(millis() / 1000);
  }
  
  char buf[64];
  size_t written = strftime(buf, 64, "%a, %Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buf, written);
}

String postData() {
  String data;
  data.reserve(1024);
  data += "{\"args\": [\"esp32test.rrd\", \"N:";
  data += sensorData.duration;
  data += ":";
  data += sensorData.voltage;
  for (const double &v : {sensorData.sht30Temperature, sensorData.sht30Humidity})
  {
    data += ":";
    if (std::isnan(v))
      data += "U";
    else
      data += v;
  }
  for (int i = 0; i < 3; ++i)
  {
    data += ":";
    if (sensorData.ds18b20.size() > i)
      data += sensorData.ds18b20[i];
    else
      data += "U";
  }
  data += "\"]}";

  return data;
}

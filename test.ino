
/* Required Libraries:
https://github.com/Xinyuan-LilyGO/TTGO-T-Display /TFT_eSPI
https://github.com/PaulStoffregen/OneWire
https://github.com/milesburton/Arduino-Temperature-Control-Library
https://github.com/Risele/SHT3x
*/
#include "HttpPostTask.h"
#include "JetBrainsMono15.h"
#include "TFT.h"
#include "WIFI_Data.h"

#include <DallasTemperature.h>
#include <esp_adc_cal.h>
#include <esp_log.h>
#include <esp_sntp.h>
#include <OneWire.h>
#include <SHT3x.h>
#include <WiFi.h>

#include <vector>

#define ADC_PIN 34
#define ONE_WIRE_BUS 17
#define BUTTON_1 0
#define BUTTON_2 35

TFT tft;
SHT3x Sht30Sensor;
int vref = 1100;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
int numberOfDevices;
DeviceAddress tempDeviceAddress;

const char* ntpServer = "fritz.box";
const char* time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";  // TimeZone rule for Europe/Rome including daylight adjustment rules (optional)

struct SensorData {
  unsigned long duration;
  double voltage;
  double sht30Temperature;
  double sht30Humidity;
  std::vector<double> ds18b20;
};

String postData();
bool isWifiConnected();
SensorData sensorData;
HttpPostTask httpPostTask(1000, isWifiConnected, postData);

void setup(void) {
  Serial.begin(115200);
  Serial.println();

  tft.init();
  tft.loadFont(JetBrainsMono15);
  tft.setRotation(3);
  tft.fillScreen(Color::Black);

  tft.drawString("Init ADC...               ", {0, 0});
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

  tft.drawString("Init SHT30 Sensor...   ", {0, 0});
  Sht30Sensor.Begin();

  tft.drawString("Init Temp Sensors...      ", {0, 0});
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
    }
  }


  tft.drawString("Init Buttons...           ", {0, 0});
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);

  tft.drawString("Connect WIFI...           ", {0, 0});
  sntp_servermode_dhcp(0);
  configTzTime(time_zone, ntpServer);
  
  WiFi.begin(WIFI_SSID, WIFI_PSK);

  tft.drawString("Initialization completed. ", {0, 0});
  tft.fillScreen(Color::Black);

  httpPostTask.start();
}

wl_status_t lastWifiStatus = WL_IDLE_STATUS;

unsigned long lastDuration = 0;
unsigned long lastDelay = 0;
unsigned long lastStart = 0;
void loop() {
  tft.drawRect(tft.size() - Vector2i{1,1}, Vector2i{1,1}, Color::White);
  unsigned long start = millis();
  unsigned long loopTime = start - lastStart;
  lastStart = start;
  String interval = "Itr: " + String(loopTime) + " ms  ";
  String fps = "FPS: " + String(1000.0 / loopTime);
  String lastDurationString = "Dur: " + String(lastDuration) + " ms  ";

  String time = getTime() + "               ";
  //String time = String(esp_log_timestamp() / 1000);

  uint16_t v = analogRead(ADC_PIN);
  sensorData.voltage = v / 4095.0 * 2.0 * 3.3 * (vref / 1000.0);
  String voltageString = "VCC: " + String(sensorData.voltage) + " V  ";

  wl_status_t wifiStatus = WiFi.status();
  String wifiStatusString = "WIFI:" + wifiStatusToString(wifiStatus) + "             ";
  if (wifiStatus != lastWifiStatus && wifiStatus == WL_CONNECTED) {
  }
  lastWifiStatus = wifiStatus;

  Sht30Sensor.UpdateData();
  uint8_t sensorError = Sht30Sensor.GetError();
  String sht30TemperatureString;
  String sht30HumidityString;
  if (sensorError) {
    sensorData.sht30Temperature = NAN;
    sensorData.sht30Humidity = NAN;
    sht30TemperatureString = "Tmp: Error " + String(sensorError) + "    ";
    sht30HumidityString = "Hum: Error " + String(sensorError) + "    ";
  } else {
    sensorData.sht30Temperature = Sht30Sensor.GetTemperature();
    sensorData.sht30Humidity = Sht30Sensor.GetRelHumidity();
    sht30TemperatureString = "Tmp: " + String(sensorData.sht30Temperature) + " °C  ";
    sht30HumidityString = "Hum: " + String(sensorData.sht30Humidity) + " %  ";
  }

  std::vector<String> ds18b20Strings;
  sensors.requestTemperatures();
  for (int i=0; i < numberOfDevices; i++) {
    if (sensors.getAddress(tempDeviceAddress, i)) {
      float tempC = sensors.getTempC(tempDeviceAddress);
      sensorData.ds18b20.emplace_back(tempC);
      ds18b20Strings.emplace_back("Tm:" + String(i) + " " + String(tempC) + " °C");
    }
  }

  String button1 = "BT1: " + String(digitalRead(BUTTON_1) ? "Up     " : "Down ");
  String button2 = "BT2: " + String(digitalRead(BUTTON_2) ? "Up     " : "Down ");

  String ip = WiFi.localIP().toString() + "   ";

  tft.setRotation(3);
  tft.setTextColor(Color::White, Color::Black);
  tft.drawRect(tft.size() - Vector2i{1,1}, Vector2i{1,1}, Color::Black);
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
  tft.drawString(button1, pos+=shift);
  tft.drawString(button2, pos+=shift);
  tft.drawString(ip, pos+=shift);

  //if (wifiStatus == WL_CONNECTED) {
  //  int httpResponseCode = sendData(lastDuration, voltage, sht30Temperature, sht30Humidity, sht30Values);
  //  tft.drawString(httpResponseCode, tft.size() - Vector2i{-50, 0});
  //}

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

String wifiStatusToString(wl_status_t status) {
  switch (status) {
    case WL_NO_SHIELD:       return "NoShield";
    case WL_IDLE_STATUS:     return "Idle";
    case WL_NO_SSID_AVAIL:   return "NoSsidAvail";
    case WL_SCAN_COMPLETED:  return "ScanCompleted";
    case WL_CONNECTED:       return "Connected";
    case WL_CONNECT_FAILED:  return "ConnectFailed";
    case WL_CONNECTION_LOST: return "ConnectionLost";
    case WL_DISCONNECTED:    return "Disconnected";
    default:                 return "InvalidState";
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

bool isWifiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

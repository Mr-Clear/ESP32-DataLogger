
/* Required Libraries:
https://github.com/Xinyuan-LilyGO/TTGO-T-Display /TFT_eSPI
https://github.com/PaulStoffregen/OneWire
https://github.com/milesburton/Arduino-Temperature-Control-Library
https://github.com/Risele/SHT3x
*/

#include "JetBrainsMono15.h"
#include "TFT.h"
#include "WIFI_Data.h"

#include <DallasTemperature.h>
#include <esp_adc_cal.h>
#include <esp_log.h>
#include <esp_sntp.h>
#include <HTTPClient.h>
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

void setup(void) {
  Serial.begin(115200);

  tft.init();
  tft.loadFont(JetBrainsMono15);
  tft.setRotation(3);
  tft.fillScreen(Color::Black);

  tft.drawString("Init ADC...               ", 0, 0);
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

  tft.drawString("Init SHT30 Sensor...   ", 0, 0);
  Sht30Sensor.Begin();

  tft.drawString("Init Temp Sensors...      ", 0, 0);
  sensors.begin();
  numberOfDevices = sensors.getDeviceCount();
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(numberOfDevices, DEC);
  Serial.println(" devices.");
  for(int i=0;i<numberOfDevices; i++){
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i)){
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


  tft.drawString("Init Buttons...           ", 0, 0);
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);

  tft.drawString("Connect WIFI...           ", 0, 0);
  sntp_servermode_dhcp(0);
  configTzTime(time_zone, ntpServer);
  
  WiFi.begin(WIFI_SSID, WIFI_PSK);

  tft.drawString("Initialization completed. ", 0, 0);
  tft.fillScreen(Color::Black);
}

wl_status_t lastWifiStatus = WL_IDLE_STATUS;

unsigned long lastDuration = 0;
unsigned long lastDelay = 0;
unsigned long lastStart = 0;
void loop() {
  tft.drawRect(~tft.size - Vector2i{1,1}, Vector2i{1,1}, Color::White);
  unsigned long start = millis();
  unsigned long loopTime = start - lastStart;
  lastStart = start;
  String interval = "Itr: " + String(loopTime) + " ms  ";
  String fps = "FPS: " + String(1000.0 / loopTime);
  String lastDurationString = "Dur: " + String(lastDuration) + " ms  ";

  String time = getTime() + "               ";
  //String time = String(esp_log_timestamp() / 1000);

  uint16_t v = analogRead(ADC_PIN);
  double voltage = v / 4095.0 * 2.0 * 3.3 * (vref / 1000.0);
  String voltageString = "VCC: " + String(voltage) + " V  ";

  wl_status_t wifiStatus = WiFi.status();
  String wifiStatusString = "WIFI:" + wifiStatusToString(wifiStatus) + "             ";
  if (wifiStatus != lastWifiStatus && wifiStatus == WL_CONNECTED) {
  }
  lastWifiStatus = wifiStatus;

  Sht30Sensor.UpdateData();
  uint8_t sensorError = Sht30Sensor.GetError();
  double sht30Temperature = NAN;
  double sht30Humidity = NAN;
  String sht30TemperatureString;
  String sht30HumidityString;
  if (sensorError) {
    sht30TemperatureString = "Tmp: Error " + String(sensorError) + "    ";
    sht30HumidityString = "Hum: Error " + String(sensorError) + "    ";
  } else {
    sht30Temperature = Sht30Sensor.GetTemperature();
    sht30Humidity = Sht30Sensor.GetRelHumidity();
    sht30TemperatureString = "Tmp: " + String(sht30Temperature) + " °C  ";
    sht30HumidityString = "Hum: " + String(sht30Humidity) + " %  ";
  }

  std::vector<double> sht30Values;
  std::vector<String> sht30Strings;
  sensors.requestTemperatures();
  for (int i=0; i < numberOfDevices; i++) {
    if (sensors.getAddress(tempDeviceAddress, i)) {
      float tempC = sensors.getTempC(tempDeviceAddress);
      sht30Values.emplace_back(tempC);
      sht30Strings.emplace_back("Tm:" + String(i) + " " + String(tempC) + " °C");
    }
  }

  String button1 = "BT1: " + String(digitalRead(BUTTON_1) ? "Up     " : "Down ");
  String button2 = "BT2: " + String(digitalRead(BUTTON_2) ? "Up     " : "Down ");

  String ip = WiFi.localIP().toString() + "   ";

  tft.setRotation(3);
  tft.setTextColor(Color::White, Color::Black);
  tft.drawRect(~tft.size - Vector2i{1,1}, Vector2i{1,1}, Color::Black);
  //tft.fillScreen(Color::Black);

  const int dy = 16;
  int y = -dy;
  int x = 0;

  tft.drawString(time, x, y+=dy);
  tft.drawString(fps, x, y+=dy);
  tft.drawString(interval, x, y+=dy);
  tft.drawString(lastDurationString, x, y+=dy);
  tft.drawString(voltageString, x, y+=dy);
  tft.drawString(sht30TemperatureString, x, y+=dy);
  tft.drawString(sht30HumidityString, x, y+=dy);
  tft.drawString(wifiStatusString, x, y+=dy);

  y = -dy;
  tft.drawString("", x, y+=dy);
  x = tft.size.y() / 2;
  for (const String &s : sht30Strings) {
    tft.drawString(s, x, y+=dy);
  }
  tft.drawString(button1, x, y+=dy);
  tft.drawString(button2, x, y+=dy);
  tft.drawString(ip, x, y+=dy);

  if (wifiStatus == WL_CONNECTED) {
    int httpResponseCode = sendData(lastDuration, voltage, sht30Temperature, sht30Humidity, sht30Values);
    tft.drawString(ip, tft.size.y() -50, y);
  }

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

int sendData(unsigned long duration, double voltage, double sht30Temp, double sht30Hum, const std::vector<double> &ds18b20)
{
  Serial.print("HTTPClient begin... ");
  HTTPClient http;
  http.begin("https://www.klierlinge.de/rrd/update");
  String data = String("{\"args\": [\"esp32test.rrd\", \"N") +
                ":" + String(duration) +
                ":" + String(voltage) +
                ":" + (!std::isnan(sht30Temp) ? String(sht30Temp) : "U") +
                ":" + (!std::isnan(sht30Hum) ? String(sht30Hum) : "U") +
                ":" + (ds18b20.size() > 0 ? String(ds18b20[0]) : "U") +
                ":" + (ds18b20.size() > 1 ? String(ds18b20[1]) : "U") +
                ":" + (ds18b20.size() > 2 ? String(ds18b20[2]) : "U") + "\"]}";
  int httpResponseCode = http.POST(data);
  http.end();
  return httpResponseCode;
}

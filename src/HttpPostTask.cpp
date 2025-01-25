#include "HttpPostTask.h"

#include <HTTPClient.h>

#include <functional>

template <typename T>
void addValue(String &s, T value, bool isValid) {
  s += ":";
  if (isValid)
    s += value;
  else
    s += "U";
}

template <typename T>
void addValueNan(String &s, T value) {
  addValue(s, value, !std::isnan(value));
}

template <typename T>
void addValueGEZ(String &s, T value) {
  addValue(s, value, value >= 0);
}

template <typename K, typename V>
void addValueMap(String &s, const std::map<K, V> &map, const K &key) {
  if(map.count(key))
    addValue(s, map.at(key), true);
  else
    addValue(s, V{}, false);
}

HttpPostTask::HttpPostTask(std::function<bool()> getWifiConnected) :
  QueueTask(500, "HTTP POST", 8192, 10, Core::Core0),
  _getWifiConnected(getWifiConnected) { }

HttpPostTask::~HttpPostTask() = default;

void HttpPostTask::setup() {
  _httpClient.reset(new HTTPClient());
  _httpClient->setReuse(true);
}

bool HttpPostTask::handleMessage(const PostData &data) {
  bool success = false;
  if (_getWifiConnected()) {
    const String postDataSource = createPostData(data);
    Serial.println(postDataSource);
    assert(_httpClient->begin("https://www.klierlinge.de/rrd/update"));
    const int httpResponseCode = _httpClient->POST(postDataSource);
    if (httpResponseCode != 200) {
      Serial.println(postDataSource + " -> " + httpResponseCode);
      Serial.println(_httpClient->getString());
    }
    success = true;
    _httpClient->end();
  }
  return success;
}

String HttpPostTask::createPostData(const PostData &data) {
  String dataString;
  dataString.reserve(1000);
  dataString += "{\"args\": [\"dataOffice.rrd\", \"";
  if (data.timestamp.has_value())
    dataString += data.timestamp.value();
  else
    dataString += "N";
  addValueGEZ(dataString, data.duration);
  addValueNan(dataString, data.bme280Temperature);
  addValueNan(dataString, data.bme280Humidity);
  addValueNan(dataString, data.bme280SeaLevelPressure);
  addValueMap(dataString, data.ds18b20, String{"28539343d4e13cd3"}); // Intern
  addValueMap(dataString, data.ds18b20, String{"28cdc143d42956be"}); // Lang
  addValueMap(dataString, data.ds18b20, String{"28a8505704e13d44"}); // Kurz
  addValueMap(dataString, data.ds18b20, String{"28c64d5704e13d04"}); // Kurz III
  dataString += "\"]}";

  return dataString;
}

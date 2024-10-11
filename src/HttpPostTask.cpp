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
  dataString += "{\"args\": [\"dataKeller.rrd\", \"";
  if (data.timestamp.has_value())
    dataString += data.timestamp.value();
  else
    dataString += "N";
  addValueGEZ(dataString, data.duration);
  addValueNan(dataString, data.sht30Temperature);
  addValueNan(dataString, data.sht30Humidity);
  addValueNan(dataString, data.dht21Temperature);
  addValueNan(dataString, data.dht21Humidity);
  addValueMap(dataString, data.ds18b20, String{"28bff880e3e13cf3"}); // Intern
  addValueMap(dataString, data.ds18b20, String{"28425b43d4b823ba"}); // Outside
  addValueMap(dataString, data.ds18b20, String{"28d6d743d48650d6"}); // Inside 1 (Punkt)
  addValueMap(dataString, data.ds18b20, String{"286bb343d406055e"}); // Inside 2
  dataString += "\"]}";

  return dataString;
}

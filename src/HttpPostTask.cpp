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
    return true;
    if (postDataSource.length()) {
      if (_httpClient->begin("https://www.klierlinge.de/rrd/update"))
      {
        const int httpResponseCode = _httpClient->POST(postDataSource);
        if (httpResponseCode != 200) {
          Serial.println(postDataSource + " -> " + httpResponseCode);
          Serial.println(_httpClient->getString());
        }
        success = true;
        _httpClient->end();
      }
    }
  }
  return success;
}

String HttpPostTask::createPostData(const PostData &data) {
  String dataString;
  dataString.reserve(1024);
  dataString += "{\"args\": [\"dataTerrasse.rrd\", \"";
  if (data.timestamp.has_value())
    dataString += data.timestamp.value();
  else
    dataString += "N";
  addValueGEZ(dataString, data.duration);
  for (const double &v : {data.sht30Temperature, data.sht30Humidity}) {
    addValueNan(dataString, v);
  }
  addValueNan(dataString, data.ds18b20_13);
  addValueNan(dataString, data.ds18b20_16);
  dataString += "\"]}";

  return dataString;
}

#include "HttpPostTask.h"

#include <HTTPClient.h>

HttpPostTask::HttpPostTask(std::function<bool()> getWifiConnected) :
  QueueTask(20, "HTTP POST", 8192, 10, Core::Core0),
  _getWifiConnected(getWifiConnected)
{
  Serial.println("HttpPostTask::HttpPostTask");
 }

HttpPostTask::~HttpPostTask() = default;

void HttpPostTask::setup() {
  _httpClient.reset(new HTTPClient());
  _httpClient->setReuse(true);
}

bool HttpPostTask::handleMessage(const PostData &data) {
  bool success = false;
  if (_getWifiConnected()) {
    const String postDataSource = createPostData(data);
    if (postDataSource.length()) {
      _httpClient->begin("https://www.klierlinge.de/rrd/update");
      const int httpResponseCode = _httpClient->POST(postDataSource);
      if(httpResponseCode == 200) {
        success = true;
      }
      _httpClient->end();
    }
  }

  return success;
}

String HttpPostTask::createPostData(const PostData &data) {
  String dataString;
  dataString.reserve(1024);
  dataString += "{\"args\": [\"esp32test.rrd\", \"N:";
  if (data.duration < 0)
    dataString += "U";
  else
    dataString += data.duration;
  dataString += ":";
  if (std::isnan(data.voltage))
    dataString += "U";
  else
    dataString += data.voltage;
  for (const double &v : {data.sht30Temperature, data.sht30Humidity}) {
    dataString += ":";
    if (std::isnan(v))
      dataString += "U";
    else
      dataString += v;
  }
  for (int i = 0; i < 3; ++i) {
    dataString += ":";
    if (data.ds18b20.size() > i)
      dataString += data.ds18b20[i];
    else
      dataString += "U";
  }
  dataString += "\"]}";

  return dataString;
}

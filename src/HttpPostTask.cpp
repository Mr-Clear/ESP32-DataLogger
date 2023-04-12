#include "HttpPostTask.h"

#include <HTTPClient.h>

HttpPostTask::HttpPostTask(unsigned long interval, std::function<bool()> getWifiConnected) :
  LoopTask(interval, "HTTP POST", 8192, 10, Core::Core0),
  _getWifiConnected(getWifiConnected),
  _dataUpdateQueue(32)
{ }

HttpPostTask::~HttpPostTask() = default;

void HttpPostTask::setup() {
  _httpClient.reset(new HTTPClient());
  _httpClient->setReuse(true);
}

void HttpPostTask::loop() {
  if (_getWifiConnected()) {
    while (_dataUpdateQueue.messagesWaiting()) {
      _dataUpdateQueue.pop()(_data);
    }
    const String postDataSource = createPostData();
    if (postDataSource.length()) {
      delay(100);
      _httpClient->begin("https://www.klierlinge.de/rrd/update");
      const int httpResponseCode = _httpClient->POST(postDataSource);
      _httpClient->end();
    }
    _data = PostData{};
  }
}


const Queue<HttpPostTask::DataUpdate> &HttpPostTask::dataUpdateQueue() const {
  return _dataUpdateQueue;
}

String HttpPostTask::createPostData() {
  String data;
  data.reserve(1024);
  data += "{\"args\": [\"esp32test.rrd\", \"N:";
  if (_data.duration < 0)
    data += "U";
  else
    data += _data.duration;
  data += ":";
  if (std::isnan(_data.voltage))
    data += "U";
  else
    data += _data.voltage;
  for (const double &v : {_data.sht30Temperature, _data.sht30Humidity}) {
    data += ":";
    if (std::isnan(v))
      data += "U";
    else
      data += v;
  }
  for (int i = 0; i < 3; ++i) {
    data += ":";
    if (_data.ds18b20.size() > i)
      data += _data.ds18b20[i];
    else
      data += "U";
  }
  data += "\"]}";

  return data;
}

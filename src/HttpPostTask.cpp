#include "HttpPostTask.h"

#include <HTTPClient.h>

HttpPostTask::HttpPostTask(unsigned long interval, std::function<bool()> getWifiConnected, std::function<String()> getPostDataSource) :
  LoopTask(interval, "HTTP POST", 8192, 10, Core::Core0),
  _getWifiConnected(getWifiConnected),
  _postDataSource(getPostDataSource)
{ }

HttpPostTask::~HttpPostTask() = default;

void HttpPostTask::setup() {
  _httpClient.reset(new HTTPClient());
  _httpClient->setReuse(true);
}

void HttpPostTask::loop() {
  if (_getWifiConnected()) {
    const String postDataSource = _postDataSource();
    if (postDataSource.length())
    {
      delay(100);
      _httpClient->begin("https://www.klierlinge.de/rrd/update");
      const int httpResponseCode = _httpClient->POST(postDataSource);
      _httpClient->end();
    }
  }
}

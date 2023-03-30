#pragma once

#include "Task.h"

#include <WString.h>

#include <functional>
#include <memory>

class HTTPClient;

class HttpPostTask : public Task {
public:
  HttpPostTask(unsigned long interval, std::function<bool()> getWifiConnected, std::function<String()> postDataSource);
  ~HttpPostTask();

protected:
  void setup() override;
  void loop() override;

private:
  std::function<bool()> _getWifiConnected;
  std::function<String()> _postDataSource;
  std::unique_ptr<HTTPClient> _httpClient;
};

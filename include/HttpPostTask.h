#pragma once

#include <LoopTask.h>
#include <Queue.h>

#include <WString.h>

#include <cmath>
#include <functional>
#include <memory>

class HTTPClient;

class HttpPostTask : public LoopTask {
public:
  struct PostData {
    unsigned long duration = -1;
    double voltage = NAN;
    uint8_t sht30Error = 1;
    double sht30Temperature = NAN;
    double sht30Humidity = NAN;
    std::vector<double> ds18b20;
  };

  using DataUpdate = std::function<void(PostData &data)>;

  HttpPostTask(unsigned long interval, std::function<bool()> getWifiConnected);
  ~HttpPostTask();

  const Queue<DataUpdate> &dataUpdateQueue() const;

protected:
  void setup() override;
  void loop() override;

private:
  std::function<bool()> _getWifiConnected;
  std::unique_ptr<HTTPClient> _httpClient;
  PostData _data;
  Queue<DataUpdate> _dataUpdateQueue;

  String createPostData();
};

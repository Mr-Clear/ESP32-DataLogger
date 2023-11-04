#pragma once

#include <QueueTask.h>

#include <WString.h>

#include <cmath>
#include <functional>
#include <memory>

class HTTPClient;

struct PostData {
  std::optional<time_t> timestamp = {};
  unsigned long duration = -1;
  double voltage = NAN;
  uint8_t sht30Error = 1;
  double sht30Temperature = NAN;
  double sht30Humidity = NAN;
  std::vector<double> ds18b20;
};

class HttpPostTask : public QueueTask<PostData> {
public:

  using DataUpdate = std::function<void(PostData &data)>;

  HttpPostTask(std::function<bool()> getWifiConnected);
  ~HttpPostTask();

protected:
  void setup() override;
  bool handleMessage(const PostData &data) override;

private:
  std::function<bool()> _getWifiConnected;
  std::unique_ptr<HTTPClient> _httpClient;

  String createPostData(const PostData &data);
};

#pragma once

#include <QueueTask.h>

#include <WString.h>

#include <cmath>
#include <functional>
#include <memory>

class HTTPClient;

struct PostData {
  unsigned long duration = -1;
  float voltage = NAN;
  float sht30Temperature = NAN;
  float sht30Humidity = NAN;
  std::array<float, 3> ds18b20;
  uint8_t sht30Error = 1;
  std::optional<time_t> timestamp = {};
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

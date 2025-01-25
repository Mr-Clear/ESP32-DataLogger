#pragma once

#include <QueueTask.h>

#include <WString.h>

#include <cmath>
#include <functional>
#include <map>
#include <memory>
#include <optional>

class HTTPClient;

struct PostData {
  std::optional<time_t> timestamp = {};
  unsigned long duration = -1;
  float bme280Temperature = NAN;
  float bme280Humidity = NAN;
  float bme280SeaLevelPressure = NAN;
  std::map<String, float> ds18b20;
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

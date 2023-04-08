#pragma once

#include "LoopTask.h"

#include <WString.h>

class WifiKeepAliveTask : public LoopTask {
public:
  WifiKeepAliveTask();
  ~WifiKeepAliveTask();

  void connectWifi();
  bool isWifiConnected();
  String wifiStatusText();
  String localIp();

  static String wifiStatusToString(int status);

protected:
  void setup() override;
  void loop() override;

private:
  unsigned long _lastReconnect;
  unsigned long _reconnectInterval = 5000;
};

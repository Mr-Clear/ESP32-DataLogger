#pragma once

#include <LoopTask.h>
#include <Observable.h>

#include <IPAddress.h>

#include <WString.h>

class WifiKeepAliveTask : public LoopTask {
public:
  WifiKeepAliveTask();
  ~WifiKeepAliveTask();

  void connectWifi();
  bool isWifiConnected();
  String wifiStatusText();

  const Observable<int> &wifiStatus();
  const Observable<IPAddress> &localIp();

  static String wifiStatusToString(int status);

protected:
  void setup() override;
  void loop() override;

private:
  unsigned long _lastReconnect;
  unsigned long _reconnectInterval = 5000;

  Observable<int> _wifiStatus;
  Observable<IPAddress> _localIp;
};

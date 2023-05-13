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

  const Observable<int> &wifiStatus();
  const Observable<String> &wifiStatusText();
  const Observable<IPAddress> &localIp();

  static String wifiStatusToString(int status);

protected:
  void setup() override;
  void loop() override;

private:
  unsigned long _lastReconnect;
  unsigned long _reconnectInterval = 5000;

  ObservableValue<int> _wifiStatus;
  ObservableFilter<int, String> _wifiStatusText;
  ObservableValue<IPAddress> _localIp;
};

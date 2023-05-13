#include "WifiKeepAliveTask.h"

#include "WIFI_Data.h"

#include <esp_log.h>
#include <WiFi.h>

namespace {
  const auto TAG = "WifiKeepAliveTask";

  void onConnected(WifiKeepAliveTask &task, WiFiEvent_t event, WiFiEventInfo_t info) {
    ESP_LOGI(TAG, "WIFI connection established.");
  }

  void onDisconnected(WifiKeepAliveTask &task, WiFiEvent_t event, WiFiEventInfo_t info) {
    ESP_LOGW(TAG, "WIFI connection lost for reaason %d, reconnecting.", info.wifi_sta_disconnected.reason);
    task.connectWifi();
  }
}

WifiKeepAliveTask::WifiKeepAliveTask() :
  LoopTask(1000, "WIFI Keep Alive", 4096, 10, Core::Core0)
{
  _localIp = WiFi.localIP();
}

WifiKeepAliveTask::~WifiKeepAliveTask() = default;

void WifiKeepAliveTask::connectWifi() {
  ESP_LOGW(TAG, "Connecting to WIFI...");
  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PSK);
  WiFi.reconnect();
  _lastReconnect = millis();
}

bool WifiKeepAliveTask::isWifiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

String WifiKeepAliveTask::wifiStatusText() {
  return wifiStatusToString(WiFi.status());
}

const Observable<int> &WifiKeepAliveTask::wifiStatus() {
  return _wifiStatus;
}

const Observable<IPAddress> &WifiKeepAliveTask::localIp() {
  return _localIp;
}

String WifiKeepAliveTask::wifiStatusToString(int status) {
  switch (status) {
    case WL_NO_SHIELD:       return "NoShield";
    case WL_IDLE_STATUS:     return "Idle";
    case WL_NO_SSID_AVAIL:   return "NoSsidAvail";
    case WL_SCAN_COMPLETED:  return "ScanCompleted";
    case WL_CONNECTED:       return "Connected";
    case WL_CONNECT_FAILED:  return "ConnectFailed";
    case WL_CONNECTION_LOST: return "ConnectionLost";
    case WL_DISCONNECTED:    return "Disconnected";
    default:                 return "InvalidState";
  }
}

void WifiKeepAliveTask::setup() {
  WiFi.onEvent([this] (WiFiEvent_t event, WiFiEventInfo_t info) { onConnected(*this, event, info); }, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent([this] (WiFiEvent_t event, WiFiEventInfo_t info) { onDisconnected(*this, event, info); }, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  WiFi.onEvent([this] (WiFiEvent_t event, WiFiEventInfo_t info) { 
    _localIp = info.got_ip.ip_info.ip.addr;
   }, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent([this] (WiFiEvent_t event, WiFiEventInfo_t info) { 
    _localIp = 0u;
   }, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_LOST_IP);
  WiFi.onEvent([this] (WiFiEvent_t event, WiFiEventInfo_t info) { 
    _wifiStatus = WiFi.status();
   });

  connectWifi();
}

void WifiKeepAliveTask::loop() {
  unsigned long currentMillis = millis();
  if (!isWifiConnected() && (currentMillis - _lastReconnect >= _reconnectInterval)) {
    ESP_LOGW(TAG, "WIFI connection lost, reconnecting.");
    connectWifi();
  }
}

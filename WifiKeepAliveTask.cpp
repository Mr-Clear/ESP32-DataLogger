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
  Task(1000, "WIFI Keep Alive", 2048, 10, Core::Core0)
{ }

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

String WifiKeepAliveTask::localIp() {
  return WiFi.localIP().toString();
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

  connectWifi();
}

void WifiKeepAliveTask::loop() {
  return;
  unsigned long currentMillis = millis();
  if (!isWifiConnected() && (currentMillis - _lastReconnect >= _reconnectInterval)) {
    ESP_LOGW(TAG, "WIFI connection lost, reconnecting.");
    connectWifi();
  }
}

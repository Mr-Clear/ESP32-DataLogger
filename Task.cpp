#include "Task.h"

#include <Arduino.h>
#include <esp_timer.h>

Task::Task(unsigned long interval, const char* name, uint32_t stackSize, unsigned int priority, Core core) :
  _interval(interval * 1000),
  _name(name),
  _stackSize(stackSize),
  _priority(priority),
  _core(core)
{ }

void Task::start() {
  if (isRunning())
    return;

  const auto success = xTaskCreatePinnedToCore(&taskStarter, _name, _stackSize, this, _priority, &_handle, static_cast<int>(_core));
  if (success == pdPASS) {
    Serial.print(String() + "Task \"" + _name + "\" started.");
  } else {
    Serial.print(String() + "Failed to start task \"" + _name + "\". Error code: " + success);
  }

  _running = true;
  _stopped = false;
}

void Task::stop() {
  _stopped = true;
}

bool Task::isRunning() const {
  return _running;
}

void Task::taskStarter(void* task) {
  static_cast<Task*>(task)->taskLoop();
}

void Task::taskLoop() {
  setup();
  int64_t end = esp_timer_get_time();
  int64_t start;
  while (!_stopped) {
    start = esp_timer_get_time();
    _statistics.totalIdle += start - end;

    loop();

    ++_statistics.loops;
    end = esp_timer_get_time(); 
    const int64_t duration = end - start;
    _statistics.totalDuration += duration;
    const int64_t wait = _interval - duration;
    _statistics.overtime += _max(wait, 0);
    delay(constrain(wait, 0, _interval) / 1000);
  }
  _running = false;
  _stopped = true;
  vTaskDelete(_handle);
}

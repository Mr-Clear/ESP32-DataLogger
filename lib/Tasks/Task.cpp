#include "Task.h"

#include <Arduino.h>
#include <esp_timer.h>

Task::Task(const char* name, uint32_t stackSize, unsigned int priority, Core core) :
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
    Serial.println(String() + "Task \"" + _name + "\" started.");
  } else {
    Serial.println(String() + "Failed to start task \"" + _name + "\". Error code: " + success);
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
  static_cast<Task*>(task)->taskStarter();
}

void Task::taskStarter() {
  onStart();
  _running = false;
  _stopped = true;
  vTaskDelete(_handle);
}

bool Task::isStopped() {
  return _stopped;
}

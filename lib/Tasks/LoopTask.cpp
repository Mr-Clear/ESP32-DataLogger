#include "LoopTask.h"

#include <Arduino.h>
#include <esp_timer.h>

LoopTask::LoopTask(unsigned long interval, const char* name, uint32_t stackSize, unsigned int priority, Core core) :
  Task(name, stackSize, priority, core),
  _interval(interval * 1000)
{ }

void LoopTask::onStart() {
  setup();
  int64_t end = esp_timer_get_time();
  int64_t start;
  while (!isStopped()) {
    start = esp_timer_get_time();

    loop();

    end = esp_timer_get_time(); 
    const int64_t duration = end - start;
    const int64_t wait = _interval - duration;
    delay(constrain(wait, 0, _interval) / 1000);
  }
}

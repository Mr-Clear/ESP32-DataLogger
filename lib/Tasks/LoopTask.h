#pragma once

#include "Task.h"

class LoopTask : public Task {
public:
  LoopTask(unsigned long interval, const char* name, uint32_t stackSize = 4096, unsigned int priority = 10, Core core = Core::Any);

  void setInterval(unsigned long interval);
  unsigned int getInterval();

protected:
  virtual void setup() = 0;
  virtual void loop() = 0;

private:
  unsigned long _interval;
  void taskLoop();
  void onStart() override;
};

#include "FiberLoopTask.h"

FiberLoopTask::FiberLoopTask(unsigned long interval, const char* name, uint32_t stackSize, unsigned int priority, Core core) :
  LoopTask{interval, name, stackSize, priority, core}
{ }

void FiberLoopTask::setup() {
  for (Fiber *fiber : fibers()) {
    fiber->setup();
  }
}

void FiberLoopTask::loop() {
  for (Fiber *fiber : fibers()) {
    fiber->loop();
  }
}

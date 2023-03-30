#include "FiberTask.h"

FiberTask::FiberTask(unsigned long interval, const char* name, uint32_t stackSize, unsigned int priority, Core core) :
  Task{interval, name, stackSize, priority, core}
{ }

void FiberTask::addFiber(std::unique_ptr<Fiber> &fiber) {
  _fibers.emplace_back(std::move(fiber));
}

void FiberTask::setup() {
  for (std::unique_ptr<Fiber> &fiber : _fibers) {
    fiber->setup();
  }
}

void FiberTask::loop() {
  for (std::unique_ptr<Fiber> &fiber : _fibers) {
    fiber->loop();
  }
}
#include "FiberTask.h"

void FiberTask::addFiber(Fiber &fiber) {
  _fibers.emplace_back(&fiber);
}

std::vector<Fiber*> FiberTask::fibers() {
    return _fibers;
}

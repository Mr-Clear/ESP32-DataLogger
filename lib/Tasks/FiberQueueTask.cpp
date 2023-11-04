#include "FiberQueueTask.h"

FiberQueueTask::FiberQueueTask(unsigned int queSize, const char* name, uint32_t stackSize, unsigned int priority, Core core) :
  QueueTask(queSize, name, stackSize, priority, core) { 

}

void FiberQueueTask::setup() {
    for (Fiber *fiber : fibers()) {
        fiber->setup();
    }
}

bool FiberQueueTask::handleMessage(const FiberQueueTask::Callback &callback) {
    for (Fiber *fiber : fibers()) {
        fiber->loop();
    }
    callback();
    return true;
}

#include "TftTask.h"

TftTask::TftTask(unsigned int queSize, const char* name, uint32_t stackSize, unsigned int priority, Core core) :
  QueueTask(queSize, name, stackSize, priority, core) { }

void TftTask::setup() {
  _tft.init();
}

void TftTask::handleMessage(const TftTaskJob &job) {
  job(_tft);
}

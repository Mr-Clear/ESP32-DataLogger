#pragma once

#include "FiberTask.h"
#include "LoopTask.h"

class FiberLoopTask : public LoopTask, public FiberTask
{
public:
  FiberLoopTask(unsigned long interval, const char* name, uint32_t stackSize, unsigned int priority, Core core);

protected:
  void setup() override;
  void loop() override;
};

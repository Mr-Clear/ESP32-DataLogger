#pragma once

#include "Fiber.h"
#include "Task.h"

#include <vector>

class FiberTask : public Task
{
public:
  FiberTask(unsigned long interval, const char* name, uint32_t stackSize, unsigned int priority, Core core);

  void addFiber(Fiber &fiber);

protected:
  void setup() override;
  void loop() override;

private:
  std::vector<Fiber*> _fibers;
};
#pragma once

#include "Fiber.h"
#include "Task.h"

#include <memory>
#include <vector>

class FiberTask : public Task
{
public:
  FiberTask(unsigned long interval, const char* name, uint32_t stackSize, unsigned int priority, Core core);

  void addFiber(std::unique_ptr<Fiber> &fiber);

protected:
  void setup() override;
  void loop() override;

private:
  std::vector<std::unique_ptr<Fiber>> _fibers;
};
#pragma once

#include "Fiber.h"
#include "Task.h"

#include <vector>

class FiberTask
{
public:
  void addFiber(Fiber &fiber);

protected:
    std::vector<Fiber*> fibers();

private:
  std::vector<Fiber*> _fibers;
};

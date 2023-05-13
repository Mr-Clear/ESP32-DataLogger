#pragma once

class Fiber {
protected:
  virtual void setup() = 0;
  virtual void loop() = 0;
  
  friend class FiberLoopTask;
  friend class FiberQueueTask;
};

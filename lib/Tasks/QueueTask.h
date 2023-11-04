#pragma once

#include "Queue.h"
#include "Task.h"

#include <Arduino.h>

template <typename T>
class QueueTask : public Task {
public:
  QueueTask(unsigned int queSize, const char* name, uint32_t stackSize = 4096, unsigned int priority = 10, Core core = Core::Any) :
    Task(name, stackSize, priority, core),
    _queue(queSize)
  { }

  bool send(const T &data, int timeoutMs = -1) {
      return _queue.push(data, timeoutMs);
  }

protected:
  virtual void setup() = 0;
  /** @returns true when the message was handles successfully. */
  virtual bool handleMessage(const T &data) = 0;

private:
  Queue<T> _queue;
  void onStart() override {
    setup();
    while(!isStopped()) {
        if(handleMessage(_queue.peek()))
          _queue.pop();
    }
  }
};

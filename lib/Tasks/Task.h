#pragma once

#include <limits.h>
#include <stdint.h>

class tskTaskControlBlock;

class Task {
public:
  enum class Core {
    Core0 = 0,
    Core1 = 1,
    Any = 0x7FFFFFFF /* tskNO_AFFINITY */
  };

  Task(const char* name, uint32_t stackSize = 4096, unsigned int priority = 10, Core core = Core::Any);
  virtual ~Task() = default;
  void start();
  void stop();
  bool isRunning() const;

protected:
  virtual void onStart() = 0;
  bool isStopped();

private:
  const char* _name;
  uint32_t _stackSize;
  unsigned int _priority;
  Core _core;
  volatile bool _stopped;
  volatile bool _running;
  void* _handle;
  
  void taskStarter();
  static void taskStarter(void* task);
};

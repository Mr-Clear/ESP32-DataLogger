#include <limits.h>
#include <stdint.h>

class Task {
  public:
    enum class Core {
      Core0 = 0,
      Core1 = 1,
      Any = 0x7FFFFFFF /* tskNO_AFFINITY */
    };

    struct Statistics {
      int64_t minDuration = ULONG_MAX;
      int64_t maxDuration = 0;
      int64_t totalDuration = 0;
      int64_t totalIdle = 0;
      int64_t overtime = 0;
      int loops = 0;
    };

    Task(unsigned long interval, const char* name, uint32_t stackSize = 4096, unsigned int priority = 10, Core core = Core::Any);
    void start();
    void stop();
    bool isRunning() const;

  protected:
    virtual void setup() { }
    virtual void loop() { }
  
  private:
    unsigned long _interval;
    const char* _name;
    uint32_t _stackSize;
    unsigned int _priority;
    Core _core;
    volatile bool _stopped;
    volatile bool _running;
    void* _handle;
    Statistics _statistics;
    
    static void taskStarter(void* task);
    void taskLoop();
};

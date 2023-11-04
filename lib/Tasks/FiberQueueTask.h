#pragma once

#include "FiberTask.h"
#include "QueueTask.h"

#include <functional>

class FiberQueueTask : public QueueTask<std::function<void()>>, public FiberTask {
public:
    FiberQueueTask(unsigned int queSize, const char* name, uint32_t stackSize = 4096, unsigned int priority = 10, Core core = Core::Any);
    using Callback = std::function<void()>;

private:
    void setup() override;
    bool handleMessage(const Callback &callback) override;
};

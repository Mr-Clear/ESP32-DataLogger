#pragma once

#include <Tft.h>
#include <QueueTask.h>

#include <functional>

using TftTaskJob = std::function<void(Tft&)>;

class TftTask : public QueueTask<TftTaskJob> {
public:
  TftTask(unsigned int queSize, const char* name = "TFT Task", uint32_t stackSize = 8192, unsigned int priority = 10, Core core = Core::Any);

private:
  Tft _tft;
  void setup() override;
  void handleMessage(const TftTaskJob &data) override;
};

#pragma once

#include "Color.h"
#include "Vector.h"

#include <QueueTask.h>

#include <WString.h>

#include <atomic>
#include <functional>
#include <memory>
#include <sys/_stdint.h>

class TFT_eSPI;

using TftJob = std::function<void(TFT_eSPI&)>;
using Vector2i = Vector2<int>;

class Tft : public QueueTask<TftJob> {
public:
  Tft(unsigned int queSize, const char* name = "TFT Task", uint32_t stackSize = 8192, unsigned int priority = 10, Core core = Core::Any);
  ~Tft();

  const Vector2i &size();

  void setRotation(uint8_t rotation);
  uint8_t getRotation();
  void fillScreen(const Color &color);
  void drawRect(const Vector2i &topLeft, const Vector2i &size, Color color, bool filled = false);
  void drawEllipse(const Vector2i &center, const Vector2i &size, Color color, bool filled = false);
  void loadFont(const uint8_t font[]);
  void setTextColor(const Color &forground);
  void setTextColor(const Color &forground, const Color &background);
  void drawString(const char *string, const Vector2i &position);
  void drawString(const String &string, const Vector2i &position);

  uint32_t setBackLite(uint32_t level);
  uint32_t getBackLite() const;

private:
  std::unique_ptr<TFT_eSPI> _tft;
  std::atomic<uint8_t> _rotation = -1;

  const Vector2i _size{135, 240};
  const Vector2i _sizeR{~_size};

  int _backliteChannel = -1;

  void setup() override;
  void handleMessage(const TftJob &data) override;
};

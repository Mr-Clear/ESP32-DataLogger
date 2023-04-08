#pragma once

#include "Color.h"
#include "Vector.h"

#include <WString.h>

#include <memory>
#include <sys/_stdint.h>

class TFT_eSPI;

using Vector2i = Vector2<int>;

class Tft
{
public:
  Tft();
  ~Tft();

  const Vector2i &size();

  void init();
  void setRotation(uint8_t rotation);
  void fillScreen(const Color &color);
  void drawRect(const Vector2i &topLeft, const Vector2i &size, Color color, bool filled = false);
  void drawEllipse(const Vector2i &center, const Vector2i &size, Color color, bool filled = false);
  void loadFont(const uint8_t font[]);
  void setTextColor(const Color &forground);
  void setTextColor(const Color &forground, const Color &background);
  void drawString(const char *string, const Vector2i &position);
  void drawString(const String &string, const Vector2i &position);

private:
  std::unique_ptr<TFT_eSPI> _tft;

  const Vector2i _size{135, 240};
  const Vector2i _sizeR{~_size};
};

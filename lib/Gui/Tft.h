#pragma once

#include <sys/_stdint.h>

#include "Color.h"
#include "Vector.h"

#include <SPI.h>
#include <TFT_eSPI.h>

using Vector2i = Vector2<int>;

class Tft : public TFT_eSPI
{
public:
  const Vector2i &size();

  void drawRect(const Vector2i &topLeft, const Vector2i &size, Color color, bool filled = false);
  void drawEllipse(const Vector2i &center, const Vector2i &size, Color color, bool filled = false);
  void drawString(const char *string, const Vector2i &position);
  void drawString(const String &string, const Vector2i &position);

private:
  const Vector2i _size{135, 240};
  const Vector2i _sizeR{~_size};
};

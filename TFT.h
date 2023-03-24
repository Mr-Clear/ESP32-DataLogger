#include <sys/_stdint.h>

#include "Color.h"
#include "Vector.h"

#include <SPI.h>
#include <TFT_eSPI.h>

using Vector2i = Vector2<int>;

class TFT : public TFT_eSPI
{
public:
  const Vector2i size{135, 240};

  void drawRect(const Vector2i &topLeft, const Vector2i &size, Color color, bool filled = false);
  void drawEllipse(const Vector2i &center, const Vector2i &size, Color color, bool filled = false);
};

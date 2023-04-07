#include "TFT.h"

const Vector2i &TFT::size()
{
  if (getRotation() % 2)
    return _sizeR;
  return _size;
}

void TFT::drawRect(const Vector2i &topLeft, const Vector2i &size, Color color, bool filled)
{
  if (filled)
  {
    TFT_eSPI::fillRect(topLeft.x(), topLeft.y(), size.x(), size.y(), color);
  }
  else
  {
    TFT_eSPI::drawRect(topLeft.x(), topLeft.y(), size.x(), size.y(), color);
  }  
}

void TFT::drawEllipse(const Vector2i &center, const Vector2i &size, Color color, bool filled)
{
  if (filled)
  {
    TFT_eSPI::fillEllipse(center.x(), center.y(), size.x(), size.y(), color);
  }
  else
  {
    TFT_eSPI::drawEllipse(center.x(), center.y(), size.x(), size.y(), color);
  }
}

void TFT::drawString(const char *string, const Vector2i &position)
{
  TFT_eSPI::drawString(string, position.x(), position.y());
}

void TFT::drawString(const String &string, const Vector2i &position)
{
  TFT_eSPI::drawString(string, position.x(), position.y());
}
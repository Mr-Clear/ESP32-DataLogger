#include "Tft.h"

#include <SPI.h>
#include <TFT_eSPI.h>

Tft::Tft() :
  _tft(new TFT_eSPI{}) { }

Tft::~Tft() = default;

const Vector2i &Tft::size() {
  if (_tft->getRotation() % 2)
    return _sizeR;
  return _size;
}

void Tft::init() {
  _tft->init();
}

void Tft::setRotation(uint8_t rotation) {
  _tft->setRotation(rotation);
}

void Tft::fillScreen(const Color &color) {
  _tft->fillScreen(color);
}

void Tft::drawRect(const Vector2i &topLeft, const Vector2i &size, Color color, bool filled) {
  if (filled)
    _tft->fillRect(topLeft.x(), topLeft.y(), size.x(), size.y(), color);
  else
    _tft->drawRect(topLeft.x(), topLeft.y(), size.x(), size.y(), color);
}

void Tft::drawEllipse(const Vector2i &center, const Vector2i &size, Color color, bool filled) {
  if (filled)
    _tft->fillEllipse(center.x(), center.y(), size.x(), size.y(), color);
  else
    _tft->drawEllipse(center.x(), center.y(), size.x(), size.y(), color);
}

void Tft::loadFont(const uint8_t font[]) {
  _tft->loadFont(font);
}

void Tft::setTextColor(const Color &forground) {
  _tft->setTextColor(forground);
}

void Tft::setTextColor(const Color &forground, const Color &background) {
  _tft->setTextColor(forground, background);
}

void Tft::drawString(const char *string, const Vector2i &position) {
  _tft->drawString(string, position.x(), position.y());
}

void Tft::drawString(const String &string, const Vector2i &position) {
  _tft->drawString(string, position.x(), position.y());
}

#include "Tft.h"

#include <TFT_eSPI.h>

Tft::Tft(unsigned int queSize, const char* name, uint32_t stackSize, unsigned int priority, Core core) :
  QueueTask(queSize, name, stackSize, priority, core),
  _tft(new TFT_eSPI{}) {
  _rotation = _tft->getRotation();
}

Tft::~Tft() = default;

const Vector2i &Tft::size() {
  if (getRotation() % 2)
    return _sizeR;
  return _size;
}

void Tft::setRotation(uint8_t rotation) {
  send([this, rotation] (TFT_eSPI &tft) {
    tft.setRotation(rotation); 
    _rotation = rotation;
  });
  _rotation = rotation;
}

uint8_t Tft::getRotation() {
  return _rotation;
}

void Tft::fillScreen(const Color &color) {
  send([color] (TFT_eSPI &tft) { tft.fillScreen(color); });
}

void Tft::drawRect(const Vector2i &topLeft, const Vector2i &size, Color color, bool filled) {
  if (filled)
    send([topLeft, size, color] (TFT_eSPI &tft) { tft.fillRect(topLeft.x(), topLeft.y(), size.x(), size.y(), color); });
  else
    send([topLeft, size, color] (TFT_eSPI &tft) { tft.drawRect(topLeft.x(), topLeft.y(), size.x(), size.y(), color); });
}

void Tft::drawEllipse(const Vector2i &center, const Vector2i &size, Color color, bool filled) {
  if (filled)
    send([center, size, color] (TFT_eSPI &tft) { tft.fillEllipse(center.x(), center.y(), size.x(), size.y(), color); });
  else
    send([center, size, color] (TFT_eSPI &tft) { tft.drawEllipse(center.x(), center.y(), size.x(), size.y(), color); });
}

void Tft::loadFont(const uint8_t font[]) {
  send([font] (TFT_eSPI &tft) { tft.loadFont(font); });
}

void Tft::setTextColor(const Color &forground) {
  send([forground] (TFT_eSPI &tft) { tft.setTextColor(forground); });
}

void Tft::setTextColor(const Color &forground, const Color &background) {
  send([forground, background] (TFT_eSPI &tft) { tft.setTextColor(forground, background); });
}

void Tft::drawString(const char *string, const Vector2i &position) {
  send([string, position] (TFT_eSPI &tft) { tft.drawString(string, position.x(), position.y()); });
}

void Tft::drawString(const String &string, const Vector2i &position) {
  send([string, position] (TFT_eSPI &tft) { tft.drawString(string, position.x(), position.y()); });
}

void Tft::setup() {
  send([] (TFT_eSPI &tft) { tft.init(); });
}

void Tft::handleMessage(const TftJob &job) {
  job(*_tft.get());
}

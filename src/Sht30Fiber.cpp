#include "Sht30Fiber.h"

#include <SHT3x.h>

Sht30Fiber::Sht30Fiber(SensorData &data) :
  _data(data)
{ }

Sht30Fiber::~Sht30Fiber() = default;

void Sht30Fiber::setup() {
  _sensor.reset(new SHT3x);
  _sensor->Begin();
}

void Sht30Fiber::loop() {
  _sensor->UpdateData();
  const uint8_t sensorError = _sensor->GetError();
  _data.sht30Error = sensorError;
  if (sensorError) {
    _data.sht30Temperature = NAN;
    _data.sht30Humidity = NAN;
  } else {
    _data.sht30Temperature = _sensor->GetTemperature();
    _data.sht30Humidity = _sensor->GetRelHumidity();
  }
}
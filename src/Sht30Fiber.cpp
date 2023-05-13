#include "Sht30Fiber.h"

#include <SHT3x.h>

bool Sht30Fiber::Data::operator==(const Data &o) const {
  return temperature == o.temperature && humidity == o.humidity && error == o.error;
}
bool Sht30Fiber::Data::operator!=(const Data &o) const {
  return !(*this == o);
}

Sht30Fiber::Sht30Fiber() = default;
Sht30Fiber::~Sht30Fiber() = default;

const Observable<Sht30Fiber::Data> &Sht30Fiber::data(){
  return _data;
}

const Observable<float> &Sht30Fiber::temperature() {
  return _temperature;
}

const Observable<float> &Sht30Fiber::humidity(){
  return _humidity;
}
const Observable<u_int8_t> &Sht30Fiber::error() {
  return _error;
}

void Sht30Fiber::setup() {
  _sensor.reset(new SHT3x);
  _sensor->Begin();
}

void Sht30Fiber::loop() {
  _sensor->UpdateData();
  const uint8_t sensorError = _sensor->GetError();
  Data data;
  data.error = sensorError;
  if (sensorError) {
    data.temperature = NAN;
    data.humidity = NAN;
  } else {
    data.temperature = _sensor->GetTemperature();
    data.humidity = _sensor->GetRelHumidity();
  }
  _data = data;
  _temperature = data.temperature;
  _humidity = data.humidity;
  _error = data.error;
}

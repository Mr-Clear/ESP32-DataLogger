#include "Dht21Fiber.h"

#include <Adafruit_Sensor.h>
#include <DHT.h>

bool Dht21Fiber::Data::operator==(const Data &o) const {
  return temperature == o.temperature && humidity == o.humidity;
}
bool Dht21Fiber::Data::operator!=(const Data &o) const {
  return !(*this == o);
}

Dht21Fiber::Dht21Fiber(uint8_t pin) : _pin{pin} { }
Dht21Fiber::~Dht21Fiber() = default;

void Dht21Fiber::scan() {
  _sensor->begin();
}

const Observable<Dht21Fiber::Data> &Dht21Fiber::data(){
  return _data;
}

const Observable<float> &Dht21Fiber::temperature() {
  return _temperature;
}

const Observable<float> &Dht21Fiber::humidity(){
  return _humidity;
}

void Dht21Fiber::setup() {
  _sensor.reset(new DHT{_pin, DHT21});
  _sensor->begin();
}

void Dht21Fiber::loop() {
  Data data;
  data.temperature = _sensor->readTemperature();
  data.humidity = _sensor->readHumidity();
  _data = data;
  _temperature = data.temperature;
  _humidity = data.humidity;
}

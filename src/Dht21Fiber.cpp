#include "Dht21Fiber.h"

#include <DHT_U.h>

bool Dht21Fiber::Data::operator==(const Dht21Fiber::Data &other) const {
    return temperature == other.temperature
        && humidity == other.humidity;
}

bool Dht21Fiber::Data::operator!=(const Dht21Fiber::Data &other) const {
    return !operator==(other);
}

Dht21Fiber::Dht21Fiber(uint8_t pin) :
  _pin{pin}
{ }

Dht21Fiber::~Dht21Fiber() = default;

const Observable<Dht21Fiber::Data> &Dht21Fiber::data() {
    return _data;
}

void Dht21Fiber::scan() {
    _rescan = true;
}

void Dht21Fiber::setup() {
    initSensor();
}

void Dht21Fiber::loop() {
    if (_rescan) {
        initSensor();
        _rescan = false;
    }
    static ulong lastUpdate = 0;
    const ulong now = micros();
    if (now - lastUpdate < _minDelay)
        return;
    lastUpdate = now;
    Data data;
    sensors_event_t event;
    _sensor->temperature().getEvent(&event);
    data.temperature = event.temperature;
    _sensor->humidity().getEvent(&event);
    data.humidity = event.relative_humidity;
    _data = data;
}

void Dht21Fiber::initSensor() {
    _sensor.reset(new DHT_Unified(_pin, DHT21));
    sensor_t sensor;
    _sensor->temperature().getSensor(&sensor);
    _minDelay = sensor.min_delay;
}

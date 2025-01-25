#include "Bme280Fiber.h"

#include <Adafruit_BME280.h>

bool Bme280Fiber::Data::operator==(const Bme280Fiber::Data &other) const {
    return temperature == other.temperature
        && pressureSeaLevel == other.pressureSeaLevel;
}

bool Bme280Fiber::Data::operator!=(const Bme280Fiber::Data &other) const {
    return !operator==(other);
}

Bme280Fiber::Bme280Fiber()
{ }

Bme280Fiber::~Bme280Fiber() = default;

const Observable<Bme280Fiber::Data> &Bme280Fiber::data() {
    return _data;
}

void Bme280Fiber::scan() {
    _rescan = true;
}

void Bme280Fiber::setup() {
    initSensor();
}

void Bme280Fiber::loop() {
    if (_rescan) {
        initSensor();
        _rescan = false;
    }
    float pressure = _sensor->readPressure();
    Data data{_sensor->readTemperature(), _sensor->readHumidity(), _sensor->seaLevelForAltitude(577.3, pressure)};
    _data = data;
}

void Bme280Fiber::initSensor() {
    _sensor.reset(new Adafruit_BME280);
    Serial.println(String("BME280 begin: ") + _sensor->begin());
}

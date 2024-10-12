#include "Bmp180Fiber.h"

#include <Adafruit_BMP085.h>

bool Bmp180Fiber::Data::operator==(const Bmp180Fiber::Data &other) const {
    return temperature == other.temperature
        && pressureSeaLevel == other.pressureSeaLevel;
}

bool Bmp180Fiber::Data::operator!=(const Bmp180Fiber::Data &other) const {
    return !operator==(other);
}

Bmp180Fiber::Bmp180Fiber()
{ }

Bmp180Fiber::~Bmp180Fiber() = default;

const Observable<Bmp180Fiber::Data> &Bmp180Fiber::data() {
    return _data;
}

void Bmp180Fiber::scan() {
    _rescan = true;
}

void Bmp180Fiber::setup() {
    initSensor();
}

void Bmp180Fiber::loop() {
    if (_rescan) {
        initSensor();
        _rescan = false;
    }
    Data data{_sensor->readTemperature(), _sensor->readSealevelPressure(577.3)};
    _data = data;
}

void Bmp180Fiber::initSensor() {
    _sensor.reset(new Adafruit_BMP085);
    Serial.println(String("BMP180 begin: ") + _sensor->begin());
}

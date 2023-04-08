#include "Ds18b20Fiber.h"

#include <DallasTemperature.h>
#include <OneWire.h>

namespace {

String deviceAddress2String(DeviceAddress deviceAddress) {
    String s;
    s.reserve(16);
    for (uint8_t i = 0; i < 8; i++) {
        if (deviceAddress[i] < 16)
            s += "0";
        s += String(deviceAddress[i], 16);
    }
    return s;
}
}

Ds18b20Fiber::Ds18b20Fiber(uint8_t pin) :
  _pin(pin) { }
Ds18b20Fiber::~Ds18b20Fiber() = default;

const Observable<std::map<String, float>> &Ds18b20Fiber::data() {
    return _data;
}

void Ds18b20Fiber::setup() {
    _oneWire.reset(new OneWire(_pin));
    _sensors.reset(new DallasTemperature(_oneWire.get()));

    _sensors->begin();
}

void Ds18b20Fiber::loop() {
    std::map<String, float> values;
    _sensors->requestTemperatures();
    for (int i=0; i < _sensors->getDeviceCount(); i++) {
        DeviceAddress deviceAddress;
        if (_sensors->getAddress(deviceAddress, i)) {
            float tempC = _sensors->getTempC(deviceAddress);
            values[deviceAddress2String(deviceAddress)] = tempC;
        }
    }
    _data = values;
}

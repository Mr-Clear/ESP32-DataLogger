#include "Apds9930Fiber.h"

Apds9930Fiber::Apds9930Fiber()
{ }

Apds9930Fiber::~Apds9930Fiber() = default;

const Observable<float> &Apds9930Fiber::data()
{
    return _data;
}

void Apds9930Fiber::scan()
{
    _rescan = true;
}

void Apds9930Fiber::setup()
{
    _sensor.reset(new APDS9930());
    initSensor();
}

void Apds9930Fiber::loop()
{
    if (_rescan) {
        initSensor();
        _rescan = false;
    }
    float val = 0;
    _sensor->readAmbientLightLux(val);
    _data = val;
}

void Apds9930Fiber::initSensor()
{
    _sensor->init();
    _sensor->enableLightSensor(false);
}

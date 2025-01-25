#include "Dht20Fiber.h"

bool Dht20Fiber::Data::operator==(const Dht20Fiber::Data &other) const
{
    return temperature == other.temperature
        && humidity == other.humidity
        && status == other.status;
}

bool Dht20Fiber::Data::operator!=(const Dht20Fiber::Data &other) const
{
    return !operator==(other);
}

Dht20Fiber::Dht20Fiber()
{ }

Dht20Fiber::~Dht20Fiber() = default;

const Observable<Dht20Fiber::Data> &Dht20Fiber::data()
{
    return _data;
}

void Dht20Fiber::scan()
{
    _rescan = true;
}

void Dht20Fiber::setup()
{
    initSensor();
}

void Dht20Fiber::loop()
{
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
    if (_sensor) {
        data.status = _sensor->read();
        switch (data.status) {
            case DHT20_OK:
                data.temperature = _sensor->getTemperature();
                data.humidity = _sensor->getHumidity();
                _data = data;
                break;
            case DHT20_ERROR_CHECKSUM:
                Serial.println("DHT20 Error: Checksum error");
                break;
            case DHT20_ERROR_CONNECT:
                Serial.println("DHT20 Error: Connect error");
                break;
            case DHT20_MISSING_BYTES:
                Serial.println("DHT20 Error: Missing bytes");
                break;
            case DHT20_ERROR_BYTES_ALL_ZERO:
                Serial.println("DHT20 Error: All bytes read zero");
                break;
            case DHT20_ERROR_READ_TIMEOUT:
                Serial.println("DHT20 Error: Read time out");
                break;
            case DHT20_ERROR_LASTREAD:
                Serial.println("DHT20 Error: Error read too fast");
                break;
            default:
                Serial.println("DHT20 Error: Unknown error");
                break;
        }
    }
}

void Dht20Fiber::initSensor()
{
    _sensor.reset(new DHT20);
    Wire.begin();
    _sensor->begin();
}

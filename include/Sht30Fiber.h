#pragma once

#include "SensorData.h"

#include <Fiber.h>
#include <Observable.h>

#include <memory>

class SHT3x;

class Sht30Fiber : public Fiber {
public:
  struct Data {
    float temperature;
    float humidity;
    int error;
    bool operator==(const Data &o) const;
    bool operator!=(const Data &o) const;
  };

  Sht30Fiber();
  ~Sht30Fiber();

  const Observable<Data> &data();
  const Observable<float> &temperature();
  const Observable<float> &humidity();
  const Observable<uint8_t> &error();
  
protected:
  void setup() override;
  void loop() override;

private:
  std::unique_ptr<SHT3x> _sensor;
  Observable<Data> _data;
  Observable<float> _temperature;
  Observable<float> _humidity;
  Observable<uint8_t> _error;
};

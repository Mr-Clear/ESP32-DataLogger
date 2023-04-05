#pragma once

#include "Fiber.h"
#include "SensorData.h"

#include <memory>

class SHT3x;

class Sht30Fiber : public Fiber {
public:
  Sht30Fiber(SensorData &data);
  ~Sht30Fiber();
  
protected:
  void setup() override;
  void loop() override;

private:
  SensorData &_data;
  std::unique_ptr<SHT3x> _sensor;
};

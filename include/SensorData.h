#pragma once

#include <stdint.h>
#include <vector>

struct SensorData {
  unsigned long duration;
  double voltage;
  uint8_t sht30Error;
  double sht30Temperature;
  double sht30Humidity;
  std::vector<double> ds18b20;
};

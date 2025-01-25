#pragma once

#include <Fiber.h>
#include <Observable.h>
#include <WString.h>

#include <memory>
#include <map>

class Adafruit_BME280;

class Bme280Fiber : public Fiber {
public:
  struct Data {
    float temperature;
    float humidity;
    int32_t pressureSeaLevel;
    bool operator==(const Data&) const;
    bool operator!=(const Data&) const;
  };

  Bme280Fiber();
  ~Bme280Fiber();

  const Observable<Data> &data();
  void scan();
  
protected:
  void setup() override;
  void loop() override;

private:
  void initSensor();

  std::unique_ptr<Adafruit_BME280> _sensor;
  ObservableValue<Data> _data;
  ulong _minDelay = 0;
  bool _rescan = false;
};

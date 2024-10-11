#pragma once

#include <Fiber.h>
#include <Observable.h>
#include <WString.h>

#include <memory>
#include <map>

class DHT_Unified;

class Dht21Fiber : public Fiber {
public:
  struct Data {
    float temperature;
    float humidity;
    bool operator==(const Data&) const;
    bool operator!=(const Data&) const;
  };

  Dht21Fiber(uint8_t pin);
  ~Dht21Fiber();

  const Observable<Data> &data();
  void scan();
  
protected:
  void setup() override;
  void loop() override;

private:
  void initSensor();

  std::unique_ptr<DHT_Unified> _sensor;
  ObservableValue<Data> _data;
  uint8_t _pin;
  ulong _minDelay = 0;
  bool _rescan = false;
};

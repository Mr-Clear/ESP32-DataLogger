#pragma once

#include <Fiber.h>
#include <Observable.h>

#include <memory>

class DHT;

class Dht21Fiber : public Fiber {
public:
  struct Data {
    float temperature;
    float humidity;
    
    bool operator==(const Data &o) const;
    bool operator!=(const Data &o) const;
  };

  Dht21Fiber(uint8_t pin);
  ~Dht21Fiber();

  const Observable<Data> &data();
  const Observable<float> &temperature();
  const Observable<float> &humidity();
  
  void scan();
  
protected:
  void setup() override;
  void loop() override;

private:
  uint8_t _pin;
  std::unique_ptr<DHT> _sensor;
  ObservableValue<Data> _data;
  ObservableValue<float> _temperature;
  ObservableValue<float> _humidity;
  ObservableValue<uint8_t> _error;
};

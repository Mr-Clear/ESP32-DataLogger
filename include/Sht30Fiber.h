#pragma once

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
  
  void scan();
  
protected:
  void setup() override;
  void loop() override;

private:
  std::unique_ptr<SHT3x> _sensor;
  ObservableValue<Data> _data;
  ObservableValue<float> _temperature;
  ObservableValue<float> _humidity;
  ObservableValue<uint8_t> _error;
};

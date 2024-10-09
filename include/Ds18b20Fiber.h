#pragma once

#include <Fiber.h>
#include <Observable.h>
#include <WString.h>

#include <memory>
#include <map>

class DallasTemperature;
class OneWire;

class Ds18b20Fiber : public Fiber {
public:
  Ds18b20Fiber(uint8_t pin);
  ~Ds18b20Fiber();

  const Observable<std::map<String, float>> &data();
  void scan();
  
protected:
  void setup() override;
  void loop() override;

private:
  std::unique_ptr<OneWire> _oneWire;
  std::unique_ptr<DallasTemperature> _sensors;
  ObservableValue<std::map<String, float>> _data;
  uint8_t _pin;
  bool _rescan = false;
};

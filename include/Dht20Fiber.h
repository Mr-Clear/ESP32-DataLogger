#pragma once

#include <Fiber.h>
#include <Observable.h>

#include <DHT20.h>

#include <memory>

class Dht20Fiber : public Fiber
{
public:
    struct Data {
        float temperature;
        float humidity;
        int status;
        bool operator==(const Data&) const;
        bool operator!=(const Data&) const;
    };

    Dht20Fiber();
    ~Dht20Fiber();

    const Observable<Data> &data();
    void scan();

protected:
  void setup() override;
  void loop() override;

private:
  void initSensor();

  std::unique_ptr<DHT20> _sensor;
  ObservableValue<Data> _data;
  ulong _minDelay = 0;
  bool _rescan = false;
};

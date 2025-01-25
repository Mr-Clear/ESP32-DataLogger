#pragma once

#include <Fiber.h>
#include <Observable.h>

#include <APDS9930.h>

#include <memory>

// The ambient and proximity sensor labeled CJMCU

class Apds9930Fiber : public Fiber
{
public:
    Apds9930Fiber();
    ~Apds9930Fiber();

    const Observable<float> &data();
    void scan();

protected:
  void setup() override;
  void loop() override;

private:
    void initSensor();
    
    std::unique_ptr<APDS9930> _sensor;
    ObservableValue<float> _data;
    bool _rescan = false;
    int _gain = PGAIN_2X;
};

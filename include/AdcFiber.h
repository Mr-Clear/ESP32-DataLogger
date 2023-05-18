#pragma once

#include <Fiber.h>
#include <Observable.h>

#include <vector>

class AdcFiber : public Fiber {
public:
    const Observable<double> &channel(int channel);
    void removeChannel(int channel);

protected:
    void setup() override;
    void loop() override;

private:
    std::unordered_map<int, ObservableValue<double>> _channels;
    double _vref = 1.1;
};

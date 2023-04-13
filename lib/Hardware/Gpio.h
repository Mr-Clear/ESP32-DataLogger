#pragma once

#include <functional>

enum class PinInputMode {
    PullUp,
    PullDown
};

enum class PinInterruptMode {
    OnRising,
    OnFalling,
    OnChange,
    OnLow,
    OnHigh,
    OnLowWakeupEnabled,
    OnHighWakeupEnabled
};

enum class GpioEventType {
    Rising,
    Falling
};

using GpioEvent = std::function<void(uint8_t, GpioEventType)>;

void addGpioEvent(uint8_t pin, PinInputMode inputMode, GpioEvent event, int debounceTime = 100);

int reserveLedChannel();
void freeLedChannel(int channel);

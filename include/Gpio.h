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

void addGpioEvent(uint8_t pin, PinInputMode inputMode, std::function<void(void)> event, PinInterruptMode interruptMode, int debounceTime = 50);

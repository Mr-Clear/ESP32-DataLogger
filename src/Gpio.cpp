#include "Gpio.h"

#include <esp32-hal-gpio.h>
#include <FunctionalInterrupt.h>

namespace {

    uint8_t decodePinInputMode(PinInputMode mode) {
        switch (mode)
        {
        case PinInputMode::PullUp:
            return INPUT_PULLUP;
        case PinInputMode::PullDown:
            return INPUT_PULLDOWN;
        default:
            return 0;
        }
    }

    uint8_t decodePinInterruptMode(PinInterruptMode mode) {
        switch (mode)
        {
        case PinInterruptMode::OnRising:
            return RISING;
        case PinInterruptMode::OnFalling:
            return FALLING;
        case PinInterruptMode::OnChange:
            return CHANGE;
        case PinInterruptMode::OnLow:
            return ONLOW;
        case PinInterruptMode::OnHigh:
            return ONHIGH;
        case PinInterruptMode::OnLowWakeupEnabled:
            return ONLOW_WE;
        case PinInterruptMode::OnHighWakeupEnabled:
            return ONHIGH_WE;
        default:
            return 0;
        }
    }

} // namespace


void addGpioEvent(uint8_t pin, PinInputMode inputMode, std::function<void()> event, PinInterruptMode interruptMode, int debounceTime) {
    pinMode(pin, decodePinInputMode(inputMode));
    attachInterrupt(pin, [event, debounceTime] {
        static int count = 0;
        static long lastPress = 0;
        const long now = millis();
        if (now - lastPress > debounceTime)
            event();
        lastPress = now;
    }, decodePinInterruptMode(interruptMode));
}

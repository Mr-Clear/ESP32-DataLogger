#include "Gpio.h"

#include <esp32-hal-gpio.h>
#include <FunctionalInterrupt.h>

#include <set>

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

    constexpr uint8_t decodePinInterruptMode(PinInterruptMode mode) {
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

    std::set<int> freeLedChannels{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 ,15};

} // namespace

void addGpioEvent(uint8_t pin, PinInputMode inputMode, GpioEvent event, int debounceTime) {
    pinMode(pin, decodePinInputMode(inputMode));
    attachInterrupt(pin, [pin, event, debounceTime] {
        static int count = 0;
        static long lastPress = 0;
        const long now = millis();
        const GpioEventType type = digitalRead(pin) ? GpioEventType::Rising : GpioEventType::Falling;
        if (now - lastPress > debounceTime)
            event(pin, type);
        lastPress = now;
    }, decodePinInterruptMode(PinInterruptMode::OnChange));
}

int reserveLedChannel() {
    auto begin = freeLedChannels.begin();
    if (begin == freeLedChannels.end())
        return -1;
    int channel = *begin;
    freeLedChannels.erase(begin);
    return channel;
}

void freeLedChannel(int channel) {
    freeLedChannels.emplace(channel);
}

#include "AdcFiber.h"

#include <Arduino.h>
#include <esp_adc_cal.h>

const Observable<double> &AdcFiber::channel(int channel) {
    return _channels[channel];
}
void AdcFiber::removeChannel(int channel) {
    _channels.erase(channel);
}

void AdcFiber::setup() {
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);    //Check type of calibration value used to characterize ADC
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        _vref = adc_chars.vref / 1000.0;
    } else {
        _vref = 1.1;
    }
}

void AdcFiber::loop() {
    for (auto &channel : _channels) {
        channel.second = analogRead(channel.first) / 4095.0 * 3.3 * _vref;
    }
}

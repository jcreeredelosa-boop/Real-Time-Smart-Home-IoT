#pragma once

// Verify these GPIOs against your exact ESP32 board and wiring before powering.
#define DEVICE_ID "bathroom"
#define GPIO_DHT22_DATA 4
#define GPIO_PIR_OUT 27
#define GPIO_LDR_ADC1_CHANNEL ADC1_CHANNEL_6  // GPIO34 on classic ESP32

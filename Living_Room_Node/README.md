# Living_Room_Node

PlatformIO project for the **living_room** ESP32 node.

## Build
Open this folder itself in VS Code with PlatformIO installed, then run **PlatformIO: Build**.

## Current scaffold status
- Native ESP-IDF framework selected in `platformio.ini`.
- Uses `app_main()` and FreeRTOS tasks.
- PIR GPIO input initialized.
- LDR ADC1 input initialized (classic ESP32 GPIO34 mapping).
- DHT22 driver and actual temperature/humidity acquisition are not implemented yet.
- Servo task placeholder is present; PWM and MQTT command handling are not implemented yet.
- Wi-Fi, InfluxDB, MQTT, and timestamps are not implemented yet.

## Default pin plan (verify before wiring)
- DHT22 DATA: GPIO4
- PIR OUT: GPIO27
- LDR AO: GPIO34 / ADC1_CHANNEL_6
- SG90 signal: GPIO18

Do not power the servo from the ESP32 3.3 V pin. Use a suitable regulated supply and connect its GND to ESP32 GND. Confirm the adapter's voltage/current label and the servo's rated voltage before connecting.

# BCA152 Project 3 – Smart Home Logging, Monitoring, and Remote Control

This workspace is a starter structure for the four-node project described in the BCA152 Project 3 handout.

## Important
- The project requires **four physical ESP32 boards operating simultaneously**. This workspace does not replace that requirement.
- Each room is a separate PlatformIO project using **native ESP-IDF** (`framework = espidf`) and `app_main()`. It does not use Arduino.
- The current firmware is a **Stage 1 scaffold**: it initializes the PIR and LDR interfaces and starts FreeRTOS tasks. DHT22 acquisition, Wi-Fi, InfluxDB, MQTT, and final servo behavior will be added and tested in later stages.
- Confirm your exact ESP32 board and sensor module pin labels before wiring. Do not connect an unverified 5 V output to an ESP32 GPIO.
- The default GPIO assignments are starting suggestions for a classic ESP32 DevKit/WROOM board, not universal pinouts.

## Folder layout

- `Kitchen_Node/` – kitchen ESP32 PlatformIO project
- `Bathroom_Node/` – bathroom ESP32 PlatformIO project
- `Bedroom_Node/` – bedroom ESP32 PlatformIO project
- `Living_Room_Node/` – living-room project, reserved for the SG90 servo
- `Documentation/` – wiring, pin mapping, and test records
- `Grafana/` – dashboard exports/instructions added later

## Open in VS Code

1. Extract the ZIP file.
2. Open the extracted `BCA152-Smart-Home-IoT` folder in Visual Studio Code.
3. Install the PlatformIO IDE extension.
4. Open one room folder at a time as its own PlatformIO project (for example, `Kitchen_Node`).
5. In PlatformIO, build first, then connect the matching ESP32 and upload.
6. Open the PlatformIO Serial Monitor at 115200 baud.

## Initial pin plan (verify before wiring)

| Signal | GPIO | Notes |
|---|---:|---|
| DHT22 data | 4 | Requires correct pull-up arrangement; driver will be added in Stage 2 |
| PIR OUT | 27 | Confirm module output is compatible with 3.3 V GPIO |
| LDR AO | 34 | ADC1 input-only pin on classic ESP32; use only if your LDR board has AO |
| SG90 signal | 18 | Living-room node only; servo gets a separate suitable 5 V supply |

All sensor grounds must share a common ground with the ESP32. The servo supply ground must also connect to the living-room ESP32 GND. Do not power the servo from the ESP32 3.3 V pin.

## Next development stages

1. Verify board model and sensor pin labels.
2. Wire and test one node.
3. Implement DHT22 driver and validated sensor readings.
4. Add Wi-Fi and timestamp synchronization.
5. Add InfluxDB logging.
6. Add Grafana dashboard and alert.
7. Add MQTT command validation and living-room servo control.
8. Repeat for all four nodes and perform the required simultaneous test.

Never commit Wi-Fi passwords, InfluxDB tokens, or MQTT credentials. Use a local, untracked configuration file for secrets.

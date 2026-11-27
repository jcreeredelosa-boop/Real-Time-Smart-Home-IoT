# Smart Home IoT Monitoring System (ESP32 + Home Assistant + Grafana)

An IoT-based smart home monitoring system built using ESP32 development boards and the ESP-IDF framework. It monitors temperature, humidity, motion, and light conditions across four areas: Kitchen, Bedroom, Living Room, and Bathroom.

The system integrates Mosquitto MQTT, Telegraf, InfluxDB, and Grafana for real-time data communication, storage, and visualization. Home Assistant is also integrated to provide remote control of the Living Room curtain demonstration.

![framework](https://img.shields.io/badge/framework-ESP--IDF-red) ![platform](https://img.shields.io/badge/platform-ESP32-blue) ![automation](https://img.shields.io/badge/automation-Home%20Assistant-41BDF5) ![protocol](https://img.shields.io/badge/protocol-MQTT-660066) ![database](https://img.shields.io/badge/database-InfluxDB-22ADF6) ![visualization](https://img.shields.io/badge/visualization-Grafana-orange) ![hardware](https://img.shields.io/badge/hardware-DHT11%20%7C%20PIR%20%7C%20LDR-green) ![control](https://img.shields.io/badge/control-Remote%20Curtain%20Control-purple)

---

## Features

- Four-area environmental monitoring: Kitchen, Bedroom, Living Room, and Bathroom.
- Temperature and humidity monitoring using DHT11 sensors.
- Motion detection using PIR sensors.
- Light detection using LDR sensors.
- MQTT-based wireless communication.
- Real-time sensor data collection using Telegraf.
- Time-series data storage using InfluxDB 2.
- Real-time dashboards using Grafana.
- Living Room curtain control through Home Assistant.
- Docker-based backend services.

## Hardware Components

- ESP32 Development Boards (4 units)
- DHT11 Temperature and Humidity Sensors
- PIR Motion Sensors
- LDR Light Sensors
- LED (Curtain Demonstration)
- Resistors
- Breadboards and Jumper Wires

## Technologies Used

- ESP32 and ESP-IDF
- C Programming Language
- Visual Studio Code and PlatformIO
- Mosquitto MQTT Broker
- Telegraf
- InfluxDB 2
- Grafana
- Home Assistant OS
- Oracle VirtualBox
- Docker and Docker Compose

## System Communication

```text
Kitchen ESP32 ──────┐
Bedroom ESP32 ──────┤
Living Room ESP32 ──┼── MQTT ── Mosquitto
Bathroom ESP32 ─────┘                |
                                     |
                                  Telegraf
                                     |
                                  InfluxDB
                                     |
                                   Grafana

Home Assistant ───── MQTT ───── Living Room ESP32
                                  (Curtain Control)
```

## MQTT Configuration

| Topic | Purpose |
|---|---|
| `smart-home/sensors/kitchen` | Kitchen sensor data |
| `smart-home/sensors/bedroom` | Bedroom sensor data |
| `smart-home/sensors/living_room` | Living Room sensor data |
| `smart-home/sensors/bathroom` | Bathroom sensor data |
| `smart-home/living-room/curtains/set` | Receives curtain commands |
| `smart-home/living-room/curtains/status` | Publishes curtain status |

Sensor data is transmitted using MQTT in InfluxDB line protocol format.

## Installation and Setup

Clone the repository:

```bash
git clone YOUR_REPOSITORY_URL
cd YOUR_PROJECT_FOLDER
```

Open the project in Visual Studio Code with PlatformIO and ESP-IDF installed.

Configure the Wi-Fi credentials and MQTT broker address in the appropriate ESP32 firmware configuration files. Keep passwords and private credentials out of public repositories.

Build and upload the firmware:

```bash
pio run
pio run -t upload
```

Monitor the ESP32:

```bash
pio device monitor
```

Start the backend services using Docker Compose:

```bash
docker compose up -d
```

Access the services:

- **InfluxDB:** `http://localhost:8086`
- **Grafana:** `http://localhost:3000`
- **Home Assistant:** Access through its configured VirtualBox network address.

Configure the MQTT integration in Home Assistant and connect the Living Room curtain entity to the appropriate MQTT topics.

## Project Status

The system is designed to monitor four areas: Kitchen, Bedroom, Living Room, and Bathroom. Sensor readings are transmitted through MQTT, stored in InfluxDB, and visualized in Grafana. Living Room curtain control is also integrated through Home Assistant.

## Authors

**Kent Jesser Neri**

**Alwyn Jade Reuyan**

**JC Ree Redelosa**

**Project:** Smart Home IoT Monitoring System

---

*Academic Laboratory Project — IoT and Embedded Systems*

#pragma once

#define DEVICE_ID "living_room"

#define GPIO_DHT11_DATA 4
#define GPIO_PIR_OUT 27
#define GPIO_LDR_DO 26
#define GPIO_LED 25

// Wi-Fi configuration
#define WIFI_SSID "m5 wifi"
#define WIFI_PASSWORD "fK5ZpMH3zRtw!"

// MQTT configuration
#define MQTT_BROKER_URI "mqtt://192.168.1.31:1883"
#define MQTT_SENSOR_TOPIC "smart-home/sensors/living_room"
#define MQTT_COMMAND_TOPIC "smart-home/living-room/curtains/set"
#define MQTT_STATUS_TOPIC "smart-home/living-room/curtains/status"
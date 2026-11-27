
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_mac.h"

#include "nvs_flash.h"

#include "driver/gpio.h"
#include "rom/ets_sys.h"

#include "mqtt_client.h"

#include "app_config.h"

static const char *TAG = "living_room";

#define DHT11_TIMEOUT_US 100

typedef struct {
    int temperature;
    int humidity;
} dht11_data_t;

static esp_mqtt_client_handle_t mqtt_client = NULL;
static bool mqtt_connected = false;


/* ---------------- LED CONTROL ---------------- */

static void configure_led(void)
{
    gpio_config_t led_config = {
        .pin_bit_mask = (1ULL << GPIO_LED),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&led_config));

    // LED initially OFF.
    ESP_ERROR_CHECK(gpio_set_level(GPIO_LED, 0));

    ESP_LOGI(TAG, "LED configured on GPIO %d", GPIO_LED);
}


static void set_led(bool state)
{
    ESP_ERROR_CHECK(
        gpio_set_level(GPIO_LED, state ? 1 : 0)
    );

    ESP_LOGI(
        TAG,
        "Curtain demonstration LED: %s",
        state ? "ON - CLOSE CURTAINS" : "OFF - OPEN CURTAINS"
    );

    if (mqtt_connected && mqtt_client != NULL) {
        esp_mqtt_client_publish(
            mqtt_client,
            MQTT_STATUS_TOPIC,
            state ? "CLOSED" : "OPEN",
            0,
            1,
            0
        );
    }
}


/* ---------------- MQTT ---------------- */

static void mqtt_event_handler(
    void *handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void *event_data
)
{
    esp_mqtt_event_handle_t event =
        (esp_mqtt_event_handle_t)event_data;

    switch ((esp_mqtt_event_id_t)event_id) {

        case MQTT_EVENT_CONNECTED:

            mqtt_connected = true;

            ESP_LOGI(TAG, "MQTT connected!");

            ESP_LOGI(
                TAG,
                "Subscribing to: %s",
                MQTT_COMMAND_TOPIC
            );

            esp_mqtt_client_subscribe(
                event->client,
                MQTT_COMMAND_TOPIC,
                1
            );

            break;


        case MQTT_EVENT_DISCONNECTED:

            mqtt_connected = false;

            ESP_LOGW(TAG, "MQTT disconnected.");

            break;


        case MQTT_EVENT_SUBSCRIBED:

            ESP_LOGI(TAG, "MQTT subscription successful.");

            break;


        case MQTT_EVENT_DATA:

            ESP_LOGI(TAG, "MQTT message received.");

            if (
                event->topic_len ==
                    strlen(MQTT_COMMAND_TOPIC) &&
                strncmp(
                    event->topic,
                    MQTT_COMMAND_TOPIC,
                    event->topic_len
                ) == 0
            ) {

                ESP_LOGI(
                    TAG,
                    "MQTT command: %.*s",
                    event->data_len,
                    event->data
                );

                if (
                    event->data_len == strlen("CLOSE_CURTAINS") &&
                    strncmp(
                        event->data,
                        "CLOSE_CURTAINS",
                        event->data_len
                    ) == 0
                ) {

                    set_led(true);

                } else if (
                    event->data_len == strlen("OPEN_CURTAINS") &&
                    strncmp(
                        event->data,
                        "OPEN_CURTAINS",
                        event->data_len
                    ) == 0
                ) {

                    set_led(false);

                } else {

                    ESP_LOGW(
                        TAG,
                        "Unknown curtain command."
                    );
                }
            }

            break;


        case MQTT_EVENT_ERROR:

            ESP_LOGE(TAG, "MQTT error occurred.");

            break;


        default:

            break;
    }
}


static void mqtt_start(void)
{
    esp_mqtt_client_config_t mqtt_config = {
        .broker.address.uri = MQTT_BROKER_URI
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_config);

    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize MQTT client.");
        return;
    }

    ESP_ERROR_CHECK(
        esp_mqtt_client_register_event(
            mqtt_client,
            ESP_EVENT_ANY_ID,
            mqtt_event_handler,
            NULL
        )
    );

    ESP_ERROR_CHECK(
        esp_mqtt_client_start(mqtt_client)
    );

    ESP_LOGI(TAG, "MQTT client started.");
}


/* ---------------- WIFI ---------------- */

static void wifi_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data
)
{
    if (
        event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_START
    ) {

        ESP_LOGI(TAG, "Connecting to Wi-Fi...");
        esp_wifi_connect();

    } else if (
        event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_DISCONNECTED
    ) {

        ESP_LOGW(TAG, "Wi-Fi disconnected. Reconnecting...");
        mqtt_connected = false;
        esp_wifi_connect();

    } else if (
        event_base == IP_EVENT &&
        event_id == IP_EVENT_STA_GOT_IP
    ) {

        ip_event_got_ip_t *event =
            (ip_event_got_ip_t *)event_data;

        ESP_LOGI(TAG, "Wi-Fi connected!");

        ESP_LOGI(
            TAG,
            "ESP32 IP Address: " IPSTR,
            IP2STR(&event->ip_info.ip)
        );

        // Start MQTT after obtaining a Wi-Fi IP address.
        if (mqtt_client == NULL) {
            mqtt_start();
        }
    }
}


static void wifi_init(void)
{
    esp_err_t ret = nvs_flash_init();

    if (
        ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND
    ) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_event_handler,
            NULL
        )
    );

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &wifi_event_handler,
            NULL
        )
    );

    wifi_config_t wifi_config = {0};

    strncpy(
        (char *)wifi_config.sta.ssid,
        WIFI_SSID,
        sizeof(wifi_config.sta.ssid) - 1
    );

    strncpy(
        (char *)wifi_config.sta.password,
        WIFI_PASSWORD,
        sizeof(wifi_config.sta.password) - 1
    );

    wifi_config.sta.threshold.authmode =
        WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(
        esp_wifi_set_mode(WIFI_MODE_STA)
    );

    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_STA,
            &wifi_config
        )
    );

    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi initialized.");
}


/* ---------------- DHT11 ---------------- */

static bool wait_for_level(
    int pin,
    int level,
    int timeout_us
)
{
    int64_t start = esp_timer_get_time();

    while (gpio_get_level(pin) != level) {

        if (
            (esp_timer_get_time() - start) > timeout_us
        ) {
            return false;
        }
    }

    return true;
}


static bool dht11_read_bit(int *bit)
{
    if (
        !wait_for_level(
            GPIO_DHT11_DATA,
            0,
            DHT11_TIMEOUT_US
        )
    ) {
        return false;
    }

    if (
        !wait_for_level(
            GPIO_DHT11_DATA,
            1,
            DHT11_TIMEOUT_US
        )
    ) {
        return false;
    }

    int64_t high_start = esp_timer_get_time();

    if (
        !wait_for_level(
            GPIO_DHT11_DATA,
            0,
            100
        )
    ) {
        return false;
    }

    int64_t high_duration =
        esp_timer_get_time() - high_start;

    *bit = (high_duration > 40) ? 1 : 0;

    return true;
}


static bool dht11_read(dht11_data_t *data)
{
    uint8_t bytes[5] = {0};

    ESP_ERROR_CHECK(
        gpio_set_direction(
            GPIO_DHT11_DATA,
            GPIO_MODE_OUTPUT
        )
    );

    ESP_ERROR_CHECK(
        gpio_set_level(
            GPIO_DHT11_DATA,
            0
        )
    );

    vTaskDelay(pdMS_TO_TICKS(20));

    ESP_ERROR_CHECK(
        gpio_set_direction(
            GPIO_DHT11_DATA,
            GPIO_MODE_INPUT
        )
    );

    if (
        !wait_for_level(
            GPIO_DHT11_DATA,
            0,
            100
        )
    ) {
        return false;
    }

    if (
        !wait_for_level(
            GPIO_DHT11_DATA,
            1,
            100
        )
    ) {
        return false;
    }

    if (
        !wait_for_level(
            GPIO_DHT11_DATA,
            0,
            100
        )
    ) {
        return false;
    }

    for (int i = 0; i < 40; i++) {

        int bit = 0;

        if (!dht11_read_bit(&bit)) {
            return false;
        }

        bytes[i / 8] <<= 1;
        bytes[i / 8] |= bit;
    }

    uint8_t checksum =
        (uint8_t)(
            bytes[0] +
            bytes[1] +
            bytes[2] +
            bytes[3]
        );

    if (checksum != bytes[4]) {

        ESP_LOGW(TAG, "DHT11 checksum error");

        return false;
    }

    data->humidity = bytes[0];
    data->temperature = bytes[2];

    return true;
}


/* ---------------- SENSOR CONFIGURATION ---------------- */

static void configure_inputs(void)
{
    gpio_config_t pir_config = {
        .pin_bit_mask = (1ULL << GPIO_PIR_OUT),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&pir_config));


    gpio_config_t ldr_config = {
        .pin_bit_mask = (1ULL << GPIO_LDR_DO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&ldr_config));


    gpio_config_t dht_config = {
        .pin_bit_mask = (1ULL << GPIO_DHT11_DATA),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&dht_config));
}


/* ---------------- SENSOR TASK ---------------- */

/* ---------------- SENSOR TASK ---------------- */

static void sensor_task(void *arg)
{
    ESP_LOGI(
        TAG,
        "DHT11 DATA idle level: %d",
        gpio_get_level(GPIO_DHT11_DATA)
    );

    dht11_data_t dht_data;

    char mqtt_payload[160];

    while (1) {

        int pir_state =
            gpio_get_level(GPIO_PIR_OUT);

        int ldr_state =
            gpio_get_level(GPIO_LDR_DO);

        if (dht11_read(&dht_data)) {

            ESP_LOGI(
                TAG,
                "device=%s temperature=%d C humidity=%d %% "
                "pir=%d ldr=%d led=%d mqtt=%s",
                DEVICE_ID,
                dht_data.temperature,
                dht_data.humidity,
                pir_state,
                ldr_state,
                gpio_get_level(GPIO_LED),
                mqtt_connected ? "connected" : "disconnected"
            );

            if (mqtt_connected && mqtt_client != NULL) {

                snprintf(
                    mqtt_payload,
                    sizeof(mqtt_payload),
                    "environment,room=living_room "
                    "temperature=%di,humidity=%di,pir=%di,ldr=%di",
                    dht_data.temperature,
                    dht_data.humidity,
                    pir_state,
                    ldr_state
                );

                int message_id = esp_mqtt_client_publish(
                    mqtt_client,
                    MQTT_SENSOR_TOPIC,
                    mqtt_payload,
                    0,
                    1,
                    0
                );

                if (message_id >= 0) {
                    ESP_LOGI(
                        TAG,
                        "Sensor data published: %s",
                        mqtt_payload
                    );
                } else {
                    ESP_LOGE(
                        TAG,
                        "Failed to publish sensor data"
                    );
                }
            }

        } else {

            ESP_LOGW(
                TAG,
                "device=%s DHT11=READ_FAILED "
                "pir=%d ldr=%d led=%d",
                DEVICE_ID,
                pir_state,
                ldr_state,
                gpio_get_level(GPIO_LED)
            );
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}



/* ---------------- MAIN ---------------- */

void app_main(void)
{
    ESP_LOGI(TAG, "Starting Living Room Node");

    ESP_LOGI(TAG, "Sensor: DHT11");
    ESP_LOGI(TAG, "PIR GPIO: %d", GPIO_PIR_OUT);
    ESP_LOGI(TAG, "LDR DO GPIO: %d", GPIO_LDR_DO);
    ESP_LOGI(TAG, "DHT11 DATA GPIO: %d", GPIO_DHT11_DATA);
    ESP_LOGI(TAG, "LED GPIO: %d", GPIO_LED);

    configure_inputs();

    configure_led();

    wifi_init();

    xTaskCreate(
        sensor_task,
        "sensor_task",
        4096,
        NULL,
        5,
        NULL
    );
}

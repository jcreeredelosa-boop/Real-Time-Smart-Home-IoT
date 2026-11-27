#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "app_config.h"

static const char *TAG = "bathroom";

static void sensor_task(void *arg)
{
    while (1) {
        // Stage 1 scaffold:
        // PIR digital input and LDR analog input are initialized below.
        // DHT22 acquisition will be implemented and validated in the next stage.
        int pir_state = gpio_get_level(GPIO_PIR_OUT);
        int ldr_raw = adc1_get_raw(GPIO_LDR_ADC1_CHANNEL);

        ESP_LOGI(TAG, "device=%s pir=%d ldr_raw=%d dht22=NOT_IMPLEMENTED",
                 DEVICE_ID, pir_state, ldr_raw);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

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

    // Classic ESP32 ADC1 / GPIO34. If your board is not a classic ESP32,
    // verify the ADC mapping and update app_config.h.
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12));
    ESP_ERROR_CHECK(adc1_config_channel_atten(GPIO_LDR_ADC1_CHANNEL, ADC_ATTEN_DB_11));
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting bathroom node using native ESP-IDF / FreeRTOS");
    configure_inputs();

    xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 5, NULL);
}

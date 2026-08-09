#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_ble_api.h"

#define BUTTON_PIN GPIO_NUM_9
#define LED_PIN GPIO_NUM_8
#define BUZZER_PIN GPIO_NUM_10

static const char *TAG = "BLE_BUTTON";

// --- BLE настройки ---
static uint8_t adv_data[31] = {
    0x02, 0x01, 0x06,                     // Flags
    0x0F, 0x09, 'A','M','P','_','A','C','T','I','O','N','_','B','T','N' // Complete Local Name
};

// --- Обработка нажатия ---
void handle_button_press() {
    gpio_set_level(LED_PIN, 1);
    gpio_set_level(BUZZER_PIN, 1);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    gpio_set_level(LED_PIN, 0);
    gpio_set_level(BUZZER_PIN, 0);
    ESP_LOGI(TAG, "Button pressed! Beacon sent.");
}

// --- BLE инициализация ---
void ble_init() {
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BTDM));
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());
    ESP_ERROR_CHECK(esp_ble_gap_register_callback(NULL));
    
    esp_ble_gap_set_device_name("AMP_ACTION_BTN");
    esp_ble_gap_config_adv_data((esp_ble_adv_data_t *)adv_data);
    esp_ble_gap_start_advertising(NULL);
}

// --- Настройка пинов ---
void gpio_init() {
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << BUTTON_PIN) | (1ULL << LED_PIN) | (1ULL << BUZZER_PIN);
    io_conf.mode = GPIO_MODE_INPUT_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);
}

// --- Основная задача ---
void button_task(void *pvParameter) {
    while (1) {
        if (gpio_get_level(BUTTON_PIN) == 0) {
            vTaskDelay(100 / portTICK_PERIOD_MS);
            if (gpio_get_level(BUTTON_PIN) == 0) {
                handle_button_press();
                while (gpio_get_level(BUTTON_PIN) == 0) {
                    vTaskDelay(10 / portTICK_PERIOD_MS);
                }
                vTaskDelay(200 / portTICK_PERIOD_MS);
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

// --- Точка входа ---
extern "C" void app_main() {
    gpio_init();
    ble_init();
    xTaskCreate(button_task, "button_task", 4096, NULL, 5, NULL);
    ESP_LOGI(TAG, "ESP32-C6 BLE Button ready. Waiting for press...");
#include <Arduino.h>
#include <ESP_I2S.h>
#include <esp_log.h>

#include "gpio_task.h"
#include "adc_handler.h"

static const char   *TAG_MAIN = "MAIN";
const char* identify_board(uint16_t voltage_mV);

void setup() {
    ESP_LOGI(TAG_MAIN, "Starting setup...");

    // Setup Button GPIO task
    ESP_LOGI(TAG_MAIN, "Setting up GPIO task...");
    if(setup_gpio_task()){
        ESP_LOGI(TAG_MAIN, "GPIO task created successfully.");
    } else {
        ESP_LOGE(TAG_MAIN, "Failed to create GPIO task!");
    }

    // Setup ADC 
    ESP_LOGI(TAG_MAIN, "Setting up ADC...");
    if (setup_adc() == false) {
        ESP_LOGE(TAG_MAIN, "Failed to setup ADC!");
    } else {
        // ID the board based on voltage
        ESP_LOGI(TAG_MAIN, "Identified board: %s", get_board_id());
    }

    ESP_LOGI(TAG_MAIN, "Setup succesfully completed.");
}

void loop() {}



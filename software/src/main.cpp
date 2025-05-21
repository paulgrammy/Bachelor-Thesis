#include <Arduino.h>
#include <ESP_I2S.h>
#include <esp_log.h>

#include "gpio_task.h"
#include "adc_handler.h"
#include "switch_handler.h"
#include "driver/uart.h"

static const char   *TAG_MAIN = "MAIN";

void setup() {
    ESP_LOGI(TAG_MAIN, "Starting setup...");

    // Setup ADC 
    ESP_LOGI(TAG_MAIN, "Setting up ADC...");
    if (setup_adc() == false) {
        ESP_LOGE(TAG_MAIN, "Failed to setup ADC!");
    } else {
        // ID the board based on voltage
        ESP_LOGI(TAG_MAIN, "Identified board: %s", get_board_id());
    }

    // Setup Button GPIO task
    ESP_LOGI(TAG_MAIN, "Setting up GPIO task...");
    if(setup_gpio_task()){
        ESP_LOGI(TAG_MAIN, "GPIO task created successfully.");
    } else {
        ESP_LOGE(TAG_MAIN, "Failed to create GPIO task!");
    }

    ESP_LOGI(TAG_MAIN, "Setup succesfully completed.");

    // Disable UART0 to free GPIO1
    uart_driver_delete(UART_NUM_0);
    gpio_reset_pin(GPIO_NUM_1);

    // // Setup Bypass GPIO task
    // ESP_LOGI(TAG_MAIN, "Setting up Bypass task...");
    // if(setup_bypass_task()){
    //     ESP_LOGI(TAG_MAIN, "Bypass task created successfully.");
    // } else {
    //     ESP_LOGE(TAG_MAIN, "Failed to create Bypass task!");
    // }
}

void loop() {}



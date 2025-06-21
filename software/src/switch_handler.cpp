#include "switch_handler.h"

// declarations
static const char *TAG_BYPASS_TASK = "BYPASS_TASK"; // Tag for logging
static TaskHandle_t bypassTaskHandle = NULL;       // Task handle for the bypass task
static uint8_t task_parameters;                    // Task parameters, unused but still defined
static const gpio_num_t bypass_pin = (gpio_num_t)BYPASS_PIN; // GPIO pin for the button
static const gpio_num_t hi_lo_pin = (gpio_num_t)HI_LO_PIN; // GPIO pin for the high/low signal output pin
static const gpio_num_t bypass_led_pin = (gpio_num_t)BYPASS_LED_PIN; // GPIO pin for the bypass LED

// definitions

bool setup_bypass_task(void)
{
    bool isSuccesful = false;

    ESP_LOGI(TAG_BYPASS_TASK, "Setting up Bypass task...");
    gpio_set_direction(bypass_pin, GPIO_MODE_INPUT);            // Set the GPIO pin as input
    gpio_set_pull_mode(bypass_pin, GPIO_PULLUP_ONLY);           // Set the GPIO pin to pull-up mode
    gpio_set_direction(hi_lo_pin, GPIO_MODE_OUTPUT);            // Setup Pin as outout

    xTaskCreate(
        bypass_event_loop,
        "Bypass Task",
        GPIO_STACK_SIZE,
        &task_parameters,
        TASK_PRIORITY_SWITCH,
        &bypassTaskHandle);

    if (bypassTaskHandle == NULL)                               // Check if the task has been created
    {
        isSuccesful = false;

        ESP_LOGE(TAG_BYPASS_TASK, "Failed to create Bypass button task!");
    }
    else
    {
        isSuccesful = true;

        ESP_LOGI(TAG_BYPASS_TASK, "Bypass button task created successfully!");
    }

    return isSuccesful;
}

// private

void bypass_event_loop(void *pvParameters)
{
    ESP_LOGI(TAG_BYPASS_TASK, "Bypass event Loop started...");

    bool last_button_state = true;                              // Default HIGH (pull-up)
    bool hi_lo_state = false;                                   // Track current hi_lo pin state

    while (true)
    {
        bool current_button_state = gpio_get_level(bypass_pin); // Read button on IO Pin 5 (PULL-UP)

        if (!current_button_state && last_button_state)         // HIGH to LOW transition = button press
        {
            hi_lo_state = !hi_lo_state;                         // Toggle hi_lo pin state
            gpio_set_level(hi_lo_pin, hi_lo_state ? 1 : 0);     // Set hi_lo pin
            ESP_LOGI(TAG_BYPASS_TASK, "Button pressed. hi_lo pin toggled to: %d", hi_lo_state);
        }
        else if (current_button_state && !last_button_state)    // LOW to HIGH transition = button release
        {
            ESP_LOGI(TAG_BYPASS_TASK, "Button released.");
        }

        //Turn on LED if pin is HI
        if (hi_lo_state) 
        {
            gpio_set_level(bypass_led_pin, 1);                      // Turn on LED
        }
        else 
        {
            gpio_set_level(bypass_led_pin, 0);                      // Turn off LED
        }     

        last_button_state = current_button_state;               // Update last button state

        vTaskDelay(pdMS_TO_TICKS(50));                          // Debounce delay
    }
}



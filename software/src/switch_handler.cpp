// #include "switch_handler.h"

// // declarations
// static const char *TAG_BYPASS_TASK = "BYPASS_TASK"; // Tag for logging
// static TaskHandle_t bypassTaskHandle = NULL;       // Task handle for the bypass task
// static uint8_t task_parameters;                    // Task parameters, unused but still defined
// static const gpio_num_t bypass_pin = (gpio_num_t)BYPASS_PIN; // GPIO pin for the button, default is 25
// static const gpio_num_t hi_lo_pin = (gpio_num_t)HI_LO_PIN; // GPIO pin for the high/low signal output pin

// // definitions

// bool setup_bypass_task(void)
// {
//     bool isSuccesful = false;

//     ESP_LOGI(TAG_BYPASS_TASK, "Setting up Bypass task...");
//     gpio_set_direction(bypass_pin, GPIO_MODE_INPUT);            // Set the GPIO pin as input
//     gpio_set_pull_mode(bypass_pin, GPIO_PULLUP_ONLY);           // Set the GPIO pin to pull-up mode
//     gpio_set_direction(hi_lo_pin, GPIO_MODE_OUTPUT);            // Setup Pin as outout

//     xTaskCreate(
//         bypass_event_loop,
//         "Bypass Task",
//         GPIO_STACK_SIZE,
//         &task_parameters,
//         TASK_PRIORITY_SWITCH,
//         &bypassTaskHandle);

//     if (bypassTaskHandle == NULL)                                     // Check if the task has been created
//     {
//         isSuccesful = false;

//         ESP_LOGE(TAG_BYPASS_TASK, "Failed to create Bypass button task!");
//     }
//     else
//     {
//         isSuccesful = true;

//         ESP_LOGI(TAG_BYPASS_TASK, "Bypass button task created successfully!");
//     }

//     return isSuccesful;
// }

// // private

// // poll only short press
// void bypass_event_loop(void *pvParameters)
// {
//     ESP_LOGI(TAG_BYPASS_TASK, "Bypass event Loop started...");

//     bool bypass_state = false; // Bypass state
//     bool last_bypass_state = true; // Default HIGH (pull-up)

//     while (true)
//     {
//         // bool current_bypass_state = gpio_get_level(bypass_pin); // PULL-UP => true = not pressed

//         // if (!current_bypass_state && last_bypass_state)         // HIGH to LOW transition = detected press
//         // {
//         //     bypass_state = !bypass_state; // Toggle bypass state
//         //     ESP_LOGI(TAG_BYPASS_TASK, "Bypass state changed: %d", bypass_state);

//         //     if (bypass_state)
//         //     {
//         //         gpio_set_level(hi_lo_pin, 1); // Set pin high
//         //     }
//         //     else
//         //     {
//         //         gpio_set_level(hi_lo_pin, 0); // Set pin low
//         //     }
//         // }

//         // last_bypass_state = current_bypass_state; // Update last button state

//         vTaskDelay(pdMS_TO_TICKS(50)); // Delay to avoid bouncing
//     }
// }
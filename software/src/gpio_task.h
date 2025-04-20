#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"

#define GPIO_STACK_SIZE 2048                                    // Stack size for the task
#define TASK_PRIORITY 5                                         // Task priority
#define BUTTON_PIN 25                                           // GPIO pin for the button

// public

/* Setup GPIO task function
 * This function will set up the GPIO task for the button press.
 * It will create a task that will run in the background and check for GPIO events.
 */
bool setup_gpio_task(void);                                     // Function to set up GPIO task

// private
static const char *TAG_BUTTON_TASK = "GPIO_TASK";
static uint8_t task_parameters;                                 // Task parameters, unused but still defined
static TaskHandle_t gpioHandle = NULL;                          // When task created succesfully, this will hold the address in memory of the task
const gpio_num_t button_pin = (gpio_num_t)BUTTON_PIN;           // GPIO pin for the button, default is 25

/* GPIO event loop function
 * This function is called from the setup_gpio_task() function. It will create a task that will run in the background and check for GPIO events.
 * The task will be created with a stack size of 2048 bytes and a priority of 5.
 * The task will run indefinitely until it is deleted or the ESP32 is reset.
 */
static void gpio_event_loop(void *pvParameters);

// definitions
bool setup_gpio_task(void)
{
    bool isSuccesful = false;

    ESP_LOGI(TAG_BUTTON_TASK, "Setting up GPIO task...");
    gpio_set_direction(button_pin, GPIO_MODE_INPUT);            // Set the GPIO pin as input
    gpio_set_pull_mode(button_pin, GPIO_PULLUP_ONLY);           // Set the GPIO pin to pull-up mode

    xTaskCreate(gpio_event_loop,
                "GPIO Task",
                GPIO_STACK_SIZE,
                &task_parameters,
                TASK_PRIORITY,
                &gpioHandle);                                   // Create the task with the function gpio_event_loop() and pass the task parameters to it

    if (gpioHandle == NULL)
    {
        ESP_LOGE(TAG_BUTTON_TASK, "Failed to create GPIO task!");
        isSuccesful = false;
    }
    else
    {
        ESP_LOGI(TAG_BUTTON_TASK, "GPIO task created successfully.");
        isSuccesful = true;
    }

    return isSuccesful;                                         // Return true if the task was created successfully, false otherwise
}

static void gpio_event_loop(void *pvParameters)
{
    ESP_LOGI(TAG_BUTTON_TASK, "GPIO event Loop started...");

    TickType_t start_time = 0;                                  // Variable to store the start time of the button press
    bool button_pressed = false;                                // Flag storing the state of the button press

    while (true)
    {
        if (gpio_get_level(button_pin))
        {
            if (!button_pressed)
            {
                start_time = xTaskGetTickCount();
                button_pressed = true;

                ESP_LOGI(TAG_BUTTON_TASK, "Button pressed!");
            }
            else if (button_pressed)
            {
                TickType_t elapsed_time = xTaskGetTickCount() - start_time;
                button_pressed = false;
                ESP_LOGI(TAG_BUTTON_TASK, "Button released after %d ms", elapsed_time * portTICK_PERIOD_MS);

                // Check if the button was pressed for more than 1 second
                if (elapsed_time > pdMS_TO_TICKS(1000))
                { 
                    ESP_LOGI(TAG_BUTTON_TASK, "Button held for more than 1 second!");
                    //TODO: Add your action here for long press
                    // activate bluetooth mode, stop waveform mode
                }
                else
                {
                    ESP_LOGI(TAG_BUTTON_TASK, "Button pressed for less than 1 second!");
                    // TODO: Add your action here for short press
                    // start cycling waveforms, stop
                }
            }
        }
        else
        {
            ESP_LOGI(TAG_BUTTON_TASK, "Button not pressed!");
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Add a 100ms delay to prevent excessive CPU usage
    }
}

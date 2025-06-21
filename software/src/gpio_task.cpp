#include "gpio_task.h"

static const char *TAG_BUTTON_TASK = "GPIO_TASK";
static uint8_t task_parameters;                                 // Task parameters, unused but still defined
static TaskHandle_t gpioHandle = NULL;                          // When task created successfully, this will hold the address in memory of the task
const gpio_num_t button_pin = (gpio_num_t)BUTTON_PIN;           // GPIO pin for the button, default is 25
static const gpio_num_t status_led_pin = (gpio_num_t)STATUS_LED_PIN; // GPIO pin for the status LED

// definitions
bool setup_gpio_task(void)
{
    bool isSuccesful = false;

    ESP_LOGI(TAG_BUTTON_TASK, "Setting up GPIO task...");
    gpio_set_direction(button_pin, GPIO_MODE_INPUT);            // Set the GPIO pin as input
    gpio_set_pull_mode(button_pin, GPIO_PULLUP_ONLY);           // Set the GPIO pin to pull-up mode

    xTaskCreatePinnedToCore(gpio_event_loop,
                "GPIO Task",
                GPIO_STACK_SIZE,
                &task_parameters,
                TASK_PRIORITY,
                &gpioHandle,
                1);                                   // Create the task with the function gpio_event_loop() and pass the task parameters to it

    if (gpioHandle == NULL)                                     // Check if the task has been created
    {
        isSuccesful = false;

        ESP_LOGE(TAG_BUTTON_TASK, "Failed to create GPIO task!");
    }
    else
    {
        isSuccesful = true;

        ESP_LOGI(TAG_BUTTON_TASK, "GPIO task created successfully!");
    }
    toggle_operating_mode();                                    // Kickstart bluetooth operation
    vTaskDelay(pdMS_TO_TICKS(500));                             // Delay to allow task to initialize

    return isSuccesful;
}

static void gpio_event_loop(void *pvParameters)
{
    ESP_LOGI(TAG_BUTTON_TASK, "GPIO event Loop started...");

    TickType_t start_time = 0;
    bool button_pressed = false;                                // Button pressed state
    bool last_button_state = true;                              // Default HIGH (pull-up)
    bool blinking = false;                                      // Flag to mark blinking state
    bool has_triggered_long_press = false;                      // Flag to mark long press state

    while (true)
    {
        bool current_button_state = gpio_get_level(button_pin); // PULL-UP => true = not pressed

        if (!current_button_state && last_button_state)         // HIGH to LOW transition = detected press
        {
            start_time = xTaskGetTickCount();
            button_pressed = true;
            has_triggered_long_press = false;
            gpio_set_level(status_led_pin, 1);                      // Turn LED on to indicate button press

            ESP_LOGI(TAG_BUTTON_TASK, "Button pressed!");
        }

        // Detect release
        if (current_button_state && !last_button_state && button_pressed)
        {
            button_pressed = false;                             // Mark Button release
            blinking = false;                                   // Stop blinking
            gpio_set_level(status_led_pin, 0);                      // Turn LED off to be safe
            TickType_t elapsed_time = xTaskGetTickCount() - start_time;

            if (elapsed_time < pdMS_TO_TICKS(1000))
            {
                cycle_waveforms();                              // Cycle through waveforms

                ESP_LOGI(TAG_BUTTON_TASK, "Button released, short press!");
            }
        }

        if (button_pressed && !has_triggered_long_press)        // If button is held and hasn't triggered long press action yet
        {
            TickType_t held_time = xTaskGetTickCount() - start_time;
            if (held_time > pdMS_TO_TICKS(1000))
            {
                has_triggered_long_press = true;
                blinking = true;                                // Start blinking because button is held

                ESP_LOGI(TAG_BUTTON_TASK, "Button released, long press and hold!");
                toggle_operating_mode();                        // Toggle operating mode

            }
        }

        // Blinking logic (blinks only if blinking = true and button still held)
        static bool led_state = false;
        static TickType_t last_blink_time = 0;
        if (blinking && !current_button_state)
        {
            TickType_t now = xTaskGetTickCount();
            if ((now - last_blink_time) > pdMS_TO_TICKS(100))   // Counter for 100ms
            {
                led_state = !led_state;                         // Toggle LED state
                gpio_set_level(status_led_pin, led_state);
                last_blink_time = now;                          // Update last blink time
            }
        }

        last_button_state = current_button_state;
        vTaskDelay(pdMS_TO_TICKS(30));                          // Smoother loop
    }
}

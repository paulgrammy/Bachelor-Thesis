#include "gpio_task.h"

// private
static const char *TAG_BUTTON_TASK = "GPIO_TASK";
static uint8_t task_parameters;                                 // Task parameters, unused but still defined
static TaskHandle_t gpioHandle = NULL;                          // When task created successfully, this will hold the address in memory of the task
const  gpio_num_t button_pin = (gpio_num_t)BUTTON_PIN;          // GPIO pin for the button, default is 25

// definitions
bool setup_gpio_task(void)
{
    bool isSuccesful = false;                                   // Variable to return success or failure of task creation

    ESP_LOGI(TAG_BUTTON_TASK, "Setting up GPIO task...");
    gpio_set_direction(button_pin, GPIO_MODE_INPUT);            // Set the GPIO pin as input
    gpio_set_pull_mode(button_pin, GPIO_PULLUP_ONLY);           // Set the GPIO pin to pull-up mode

    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);           // Setup Internal LED

    xTaskCreate(gpio_event_loop,
                "GPIO Task",
                GPIO_STACK_SIZE,
                &task_parameters,
                TASK_PRIORITY,
                &gpioHandle);                                   // Create the task with the function gpio_event_loop() and pass the task parameters to it

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

    return isSuccesful;                                          // Return true if the task was created successfully, false otherwise
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
            gpio_set_level(GPIO_NUM_2, 1);                      // Turn LED on to indicate button press

            ESP_LOGI(TAG_BUTTON_TASK, "Button pressed!");
        }

        // Detect release
        if (current_button_state && !last_button_state && button_pressed)
        {
            button_pressed = false;                             // Mark Button release
            blinking = false;                                   // Stop blinking
            gpio_set_level(GPIO_NUM_2, 0);                      // Turn LED off to be safe
            TickType_t elapsed_time = xTaskGetTickCount() - start_time;

            if (elapsed_time < pdMS_TO_TICKS(1000))
            {
                //TODO: Add your short press action here!

                ESP_LOGI(TAG_BUTTON_TASK, "Button released!");
                ESP_LOGI(TAG_BUTTON_TASK, "Brief press detected!");
            }
        }

        if (button_pressed && !has_triggered_long_press)        // If button is held and hasn't triggered long press action yet
        {
            TickType_t held_time = xTaskGetTickCount() - start_time;
            if (held_time > pdMS_TO_TICKS(1000))
            {
                has_triggered_long_press = true;
                blinking = true;                                // Start blinking because button is held

                //TODO: Add your long press action here!

                ESP_LOGI(TAG_BUTTON_TASK, "Long press and hold detected!");
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
                gpio_set_level(GPIO_NUM_2, led_state);
                last_blink_time = now;                          // Update last blink time          
            }
        }

        last_button_state = current_button_state;
        vTaskDelay(pdMS_TO_TICKS(50));                          // smoother loop
    }
}


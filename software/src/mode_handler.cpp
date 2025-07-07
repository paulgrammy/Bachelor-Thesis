#include "mode_handler.h"

static const char *TAG_MODE = "MODE";                           // Tag for logging
static operating_mode_t current_mode = MODE_WAVEFORM;           // Default mode
static gpio_num_t mode_pin = (gpio_num_t)MODE_PIN;              // GPIO pin for the internal LED

// definitions
void set_operating_mode(operating_mode_t mode)
{
    current_mode = mode;                                           

    ESP_LOGI(TAG_MODE, "Operating mode set to: %d", current_mode);
}

operating_mode_t get_operating_mode(void)
{
    return current_mode;
}

void toggle_operating_mode(void)
{
    const TickType_t timeout = pdMS_TO_TICKS(1000);             // 1 second timeout

    if (current_mode == MODE_BLUETOOTH)
    {
        current_mode = MODE_WAVEFORM;

        bluetooth_mode_deinit(); 

        TickType_t start_time = xTaskGetTickCount();
        while (is_bluetooth_running())
        {
            if (xTaskGetTickCount() - start_time > timeout)
            {
                ESP_LOGE(TAG_MODE, "Timedout while waiting for Bluetooth to stop...");
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(10));                      // Wait for Bluetooth to stop
        }

        waveform_mode_init();

        gpio_set_direction(MODE_PIN, GPIO_MODE_OUTPUT);         // Set the mode pin as output
        gpio_set_level(MODE_PIN, 1);                            // Set the internal LED to HIGH

        ESP_LOGI(TAG_MODE, "Switched to WAVEFORM mode");
    }
    else
    {
        current_mode = MODE_BLUETOOTH;

        waveform_mode_deinit(); 

        TickType_t start_time = xTaskGetTickCount();
        while (is_waveform_running())
        {
            if (xTaskGetTickCount() - start_time > timeout)
            {
                ESP_LOGE(TAG_MODE, "Timedout while waiting for Waveform to stop...");
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(10));                      // Wait for Waveform to stop
        }

        bluetooth_mode_init(); 
        
        gpio_set_direction(MODE_PIN, GPIO_MODE_OUTPUT);         // Set the mode pin as output
        gpio_set_level(MODE_PIN, 0);                            // Set the internal LED to LOW

        ESP_LOGI(TAG_MODE, "Switched to BLUETOOTH mode");
    }
}

void cycle_waveforms(void)
{
    if (current_mode == MODE_WAVEFORM)
    {
        set_waveform_type((get_waveform_type() + 1) % 4);       // Cycle through waveforms

        ESP_LOGI(TAG_MODE, "Waveform type set to: %d", get_waveform_type());
    }
    else
    {
        ESP_LOGW(TAG_MODE, "Waveform mode not active, cannot cycle waveforms");
    }
}

#include "mode_handler.h"

static const char *TAG_MODE = "MODE";                             // Tag for logging
static operating_mode_t current_mode = MODE_BLUETOOTH;          // Default mode

// definitions
void set_operating_mode(operating_mode_t mode) {
    current_mode = mode;                                        // Set the operating mode
    ESP_LOGI(TAG_MODE, "Operating mode set to: %d", current_mode); // Log the current operating mode
}

operating_mode_t get_operating_mode(void) {
    return current_mode;
}

void toggle_operating_mode(void) {
    if (current_mode == MODE_BLUETOOTH) {
        current_mode = MODE_WAVEFORM;

        bluetooth_mode_deinit();         // clean up

        vTaskDelay(pdMS_TO_TICKS(300));  // adjust if needed

        if (is_bluetooth_running() == false) {
            waveform_mode_init();
            
            ESP_LOGI(TAG_MODE, "Switched to WAVEFORM mode");         
        }
    } else {
        current_mode = MODE_BLUETOOTH;

        waveform_mode_deinit();          // clean up

        vTaskDelay(pdMS_TO_TICKS(300));  // adjust if needed

        if (is_waveform_running() == false) {
            bluetooth_mode_init();       // clean up

            ESP_LOGI(TAG_MODE, "Switched to BLUETOOTH mode");
        }
    }
}

void cycle_waveforms(void) {
    if (current_mode == MODE_WAVEFORM) {
        set_waveform_type((get_waveform_type() + 1) % 4); // Cycle through waveforms

        ESP_LOGI(TAG_MODE, "Waveform type set to: %d", get_waveform_type());
    } else {
        ESP_LOGW(TAG_MODE, "Waveform mode not active, cannot cycle waveforms");
    }
}

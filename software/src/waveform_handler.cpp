#include "waveform_handler.h"

// declarations

static const char *TAG_WAVEFORM = "WAVEFORM";                     // Tag for logging
static uint8_t waveform_task_parameters;                            // Task parameters, unused but still defined
static TaskHandle_t waveformTaskHandle = NULL;                // Task handle for the waveform task
static const uint8_t I2S_SCK = 14;                               // Audio data bit clock
static const uint8_t I2S_WS = 17;                                // Audio data L&R clock
static const uint8_t I2S_SDOUT = 4;                              // ESP32 audio data output
bool is_running = false;                                      // Flag to mark if the task is running

// definitions

// public
bool is_waveform_running() {
    return is_running;                                          // Return the running flag
}

void waveform_mode_init() {
    if (waveformTaskHandle == NULL) {                           // Check if the task is already running
        xTaskCreatePinnedToCore(
            waveform_task,                                      // Task function
            "Waveform Task",                                    // Task name
            WAVEFORM_TASK_STACK_SIZE,                          // Stack size
            &waveform_task_parameters,                          // Task parameters
            WAVEFORM_TASK_PRIORITY,                             // Task priority
            &waveformTaskHandle,                                // Task handle
            0                                                  // Core ID (0 or 1)
        );
    } else {
        ESP_LOGW(TAG_WAVEFORM, "Waveform task already running");
    }

    is_running = true;                                         // Set the running flag
}

void waveform_mode_deinit() {
    if (waveformTaskHandle != NULL) {                           // Check if the task is running
        vTaskDelete(waveformTaskHandle);                        // Delete the task
        waveformTaskHandle = NULL;                              // Reset the task handle
    }

    is_running = false;                                         // Reset the running flag

    // De-initialize I2S
    esp_err_t result = i2s_driver_uninstall(I2S_NUM_0);                // Uninstall the I2S driver
    if (result == ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG_WAVEFORM, "Failed to uninstall I2S driver: %s", esp_err_to_name(result)); // Log error
    } else {
        ESP_LOGI(TAG_WAVEFORM, "I2S driver uninstalled successfully"); // Log success
    } 

    i2s_driver_uninstall(I2S_NUM_0);                           // Uninstall the I2S driver

    //Reset waveform type
    waveform_task_parameters = 0;                               // Reset the waveform type
}

void waveform_task(void *pvParameters) {
    ESP_LOGI(TAG_WAVEFORM, "Starting waveform task");

    // Optionally configure I2S pins
    i2s_pin_config_t my_pins = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_SDOUT,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = true,
    };

    esp_err_t result = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    if (result == ESP_ERR_INVALID_STATE)
    {
        ESP_LOGW(TAG_WAVEFORM, "I2S already installed");
    } else if (result == ESP_OK)
    {
        ESP_LOGI(TAG_WAVEFORM, "I2S driver installed successfully");
    } else {
        ESP_LOGE(TAG_WAVEFORM, "Failed to install I2S driver: %s", esp_err_to_name(result));
        return;
    }

    // i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &my_pins);

    const int N = 1024;
    int16_t waveform_data[N * 2];  // stereo
    float phase = 0.0f;
    float frequency = 440.0f; // A4
    float sample_rate = 44100.0f;

    while (true) {
        // waveform_type_t type = (waveform_type_t)waveform_task_parameters;

        // for (int i = 0; i < N; ++i) {
        //     float value = 0.0f;
        //     float t = (float)i / N;

        //     switch (type) {
        //         case WAVEFORM_SINE:

        //             break;
        //         case WAVEFORM_SQUARE:

        //             break;
        //         case WAVEFORM_TRIANGLE:

        //             break;
        //         case WAVEFORM_SAWTOOTH:

        //             break;
        //         default:
        //             value = 0;
        //             break;
        //     }

        //     int16_t sample = (int16_t)(value * 32767);
        //     waveform_data[2 * i]     = sample; // Left
        //     waveform_data[2 * i + 1] = sample; // Right
        // }
    
        // Write the waveform data to I2S
        size_t bytes_written;
        i2s_write(I2S_NUM_0, waveform_data, sizeof(waveform_data), &bytes_written, portMAX_DELAY);
    }

}

uint8_t get_waveform_type() {
    return waveform_task_parameters;                            // Return the current waveform type
}

void set_waveform_type(uint8_t waveform_type) {
    waveform_task_parameters = waveform_type;                    // Set the current waveform type
}
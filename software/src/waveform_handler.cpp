#include "waveform_handler.h"

// declarations

static const char *TAG_WAVEFORM = "WAVEFORM";  // Tag for logging
static uint8_t waveform_task_parameters;       // Task parameters, unused but still defined
static TaskHandle_t waveformTaskHandle = NULL; // Task handle for the waveform task
static const uint8_t I2S_SCK = 14;             // Audio data bit clock
static const uint8_t I2S_WS = 17;              // Audio data L&R clock
static const uint8_t I2S_SDOUT = 4;            // ESP32 audio data output
static bool waveform_chaged = false;           // Flag used to cleanup waveform data

// definitions

// public
bool is_waveform_running()
{
    return is_running; // Return the running flag
}

void waveform_mode_init()
{
    if (waveformTaskHandle == NULL)
    {
        // Check if the task is already running
        xTaskCreatePinnedToCore(
            waveform_task,
            "Waveform Task",
            WAVEFORM_TASK_STACK_SIZE,
            &waveform_task_parameters,
            WAVEFORM_TASK_PRIORITY,
            &waveformTaskHandle,
            0);
    }
    else
    {
        ESP_LOGW(TAG_WAVEFORM, "Waveform task already running");
    }

    is_running = true;
}

void waveform_mode_deinit()
{
    if (waveformTaskHandle != NULL)
    {
        vTaskDelete(waveformTaskHandle); // Delete the task
        waveformTaskHandle = NULL;       // Reset the task handle
    }

    is_running = false;

    // De-initialize I2S
    esp_err_t result = i2s_driver_uninstall(I2S_NUM_0); // Uninstall the I2S driver
    if (result == ESP_ERR_INVALID_STATE)
    {
        ESP_LOGE(TAG_WAVEFORM, "Failed to uninstall I2S driver: %s", esp_err_to_name(result));
    }
    else
    {
        ESP_LOGI(TAG_WAVEFORM, "I2S driver uninstalled successfully");
    }

    waveform_task_parameters = 0; // Reset the waveform type
}

void waveform_task(void *pvParameters)
{
    ESP_LOGI(TAG_WAVEFORM, "Starting waveform task");

    // Optionally configure I2S pins
    i2s_pin_config_t my_pins = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_SDOUT,
        .data_in_num = I2S_PIN_NO_CHANGE};

    // This is taken from A2DP library
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 44100,
        .bits_per_sample = (i2s_bits_per_sample_t)16,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0, // default interrupt priority
        .dma_buf_count = 4,
        .dma_buf_len = 32,
        .use_apll = false,
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 0, 0)
        .tx_desc_auto_clear =
            true, // avoiding noise in case of data unavailability
        .fixed_mclk = 0,
        .mclk_multiple = (i2s_mclk_multiple_t)0, // I2S_MCLK_MULTIPLE_DEFAULT
        .bits_per_chan = I2S_BITS_PER_CHAN_DEFAULT
#else
        .tx_desc_auto_clear =
            true // avoiding noise in case of data unavailability
#endif
    };

    esp_err_t result = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    if (result == ESP_ERR_INVALID_STATE)
    {
        ESP_LOGW(TAG_WAVEFORM, "I2S already installed");
    }
    else if (result == ESP_OK)
    {
        ESP_LOGI(TAG_WAVEFORM, "I2S driver installed successfully");
    }
    else
    {
        ESP_LOGE(TAG_WAVEFORM, "Failed to install I2S driver: %s", esp_err_to_name(result));
        return;
    }

    // i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &my_pins);

    while (true)
    {
        for (int i = 0; i < N; ++i)
        {
            /* Splitting into smaller chunks to avoid blocking the CPU
             * The waveform data is generated in chunks of N/4 samples
             * This allows for smoother operation and avoids blocking the CPU
             */
            generate_wave(i, i + N / 4); // Generate the waveform
        }
    }
}

uint8_t get_waveform_type()
{
    return waveform_task_parameters; // Return the current waveform type
}

void set_waveform_type(uint8_t waveform_type)
{
    waveform_task_parameters = waveform_type; // Set the current waveform type

    memset(waveform_data, 0, sizeof(waveform_data)); // Clear the waveform data

    ESP_LOGI(TAG_WAVEFORM, "Waveform buffer cleaned %d, %d", waveform_data[0], waveform_data[1]); // Log the change
}

// private

static void generate_wave(int start_index, int end_index)
{
    // // Generate waveform data
    // for (int i = start_index; i < end_index; ++i)
    // {
        for (int i = 0; i < N; ++i)
        {
            int16_t sample = 0; // Initialize sample to zero

            // t is N/fs where N is sample index (in this case, i) and fs is sample rate
            float t = float(i) / sample_rate;

            // phase angle required for sine and square; 2*pi*f*t
            float phase = 2.0f * M_PI * frequency * t;

            /* sinf is used because it is faster than sin
             * - if sinf is used, there is no need for type conversion
             * - it will also make code faster since it uses single-precision floating point, sin uses double-precision
             * - there won't be any loss of precision since we are using 16-bit signed int
             */
            switch (waveform_task_parameters)
            {
            case WAVEFORM_SINE:
                /* Sine wave = A*sin(2*pi*f*t) = A*sin(phase)
                 * sin(phase) is in the range [-1, 1], so this will generate a continuous value in the range [-A, A]
                 */
                sample = (int16_t)(amplitude * sinf(phase));
                break;
            case WAVEFORM_SQUARE:
                /* Square wave = A*sign(sin(2*pi*f*t)) = A*sign(phase)
                 * sinf(phase) is in the range [-1, 1], so the if statement will fix sample to either -A or A
                 */
                if (sinf(phase) > 0)
                {
                    sample = (int16_t)(amplitude); // Positive half of square wave
                }
                else
                {
                    sample = (int16_t)(-amplitude); // Negative half of square wave
                }
                break;
            case WAVEFORM_TRIANGLE:
                /* Triangle wave = A*(2/pi)*asin(sin(2*pi*f*t)) = A*(2/pi)*asin(phase)
                 * sinf(phase) is in the range [-1, 1], so this will generate a continuous value in the range [-A, A]
                 * asin(sin(phase)) is in the range [-pi/2, pi/2], so this will now map the values to [-pi/2, pi/2]
                 * (2/pi) is used to scale the value to the range [-A, A]
                 */
                sample = (int16_t)(amplitude * (2.0f / M_PI) * asinf(sinf(phase)));
                break;
            case WAVEFORM_SAWTOOTH:
                /* Sawtooth wave = A*(2/pi)*(phase - pi) = A*(2/pi)*(phase)
                 * phase is in the range [0, 2*pi], so this will generate a continuous value in the range [-A, A]
                 * (2/pi) is used to scale the value to the range [-A, A]
                 */
                sample = (int16_t)(amplitude * (2.0f * (fmodf(phase, 2.0f * M_PI) / (2.0f * M_PI)) - 1.0f));; // Sawtooth from -A to +A
                break;
            default:
                break;
            }

            // Write data to the waveform data array
            waveform_data[2 * i] = sample;     // Left channel
            waveform_data[2 * i + 1] = sample; // Right channel

            // Logging will slow down the process, so it is commented out
            // // print package
            // ESP_LOGI(TAG_WAVEFORM, "Sample %d: %d", i, sample);
        // }
    }

    // Write the waveform data to I2S
    size_t bytes_written;
    i2s_write(I2S_NUM_0, waveform_data, sizeof(waveform_data), &bytes_written, portMAX_DELAY);
}

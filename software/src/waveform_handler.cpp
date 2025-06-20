#include "waveform_handler.h"

// declarations

static const char *TAG_WAVEFORM = "WAVEFORM";  // Tag for logging
static uint8_t waveform_task_parameters;       // Task parameters, unused but still defined
static TaskHandle_t waveformTaskHandle = NULL; // Task handle for the waveform task
static const uint8_t I2S_SCK = 14;             // Audio data bit clock
static const uint8_t I2S_WS = 17;              // Audio data L&R clock
static const uint8_t I2S_SDOUT = 4;            // ESP32 audio data output

static int16_t sine_lut[LUT_SIZE];
static int16_t square_lut[LUT_SIZE];
static int16_t triangle_lut[LUT_SIZE];
static int16_t infected_lut[LUT_SIZE];

static const float sample_rate = SAMPLE_RATE;
static const float frequency = FREQUENCY;
static const float amplitude = AMPLITUDE;
static uint32_t phase = 0; 
#define WAVEFORM_NUM_SAMPLES 256
static int16_t waveform_data[WAVEFORM_NUM_SAMPLES * 2]; // Stereo (L+R) samples
static int waveform_length_bytes = WAVEFORM_NUM_SAMPLES * 2 * sizeof(int16_t); // Length of the waveform data in bytes

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

    initialize_waveform_LUTs(); // Initialize the lookup tables for waveform data

    vTaskDelay(10);

    // Kickstart generation
    set_waveform_type(waveform_task_parameters);

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
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0, // default interrupt priority
        .dma_buf_count = 4,
        .dma_buf_len = 256,
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
    
    // Main loop
    while (true)
    {
        generate_wave(); // Generate the waveform data

        size_t bytes_written;
        esp_err_t err = i2s_write(I2S_NUM_0, waveform_data, waveform_length_bytes, &bytes_written, portMAX_DELAY);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG_WAVEFORM, "I2S write failed: %s", esp_err_to_name(err));
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

    // Generate 1 full period of waveform data
    generate_wave();

    ESP_LOGI(TAG_WAVEFORM, "Waveform buffer cleaned %d, %d", waveform_data[0], waveform_data[1]); // Log the change
}

// private

static void generate_wave()
{
    // // Reset phase to zero before generating a new waveform
    // phase = 0;

    // Calculate phase increment per sample (Q16.16 fixed point)
    const uint32_t phase_inc = (uint32_t)((frequency * 65536.0f * LUT_SIZE) / sample_rate);

    for (int i = 0; i < WAVEFORM_NUM_SAMPLES; ++i)
    {
        // int16_t lut_index = (int)phase % LUT_SIZE; // Calculate the index for the lookup table
        uint16_t lut_index = (phase >> 16) % LUT_SIZE;

        //Increment phase for the lookup table
        int16_t sample = 0;

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
            sample = sine_lut[lut_index]; // Use the sine lookup table for sine wave
            break;
        case WAVEFORM_SQUARE:
            /* Square wave = A*sign(sin(2*pi*f*t)) = A*sign(phase)
             * sinf(phase) is in the range [-1, 1], so the if statement will fix sample to either -A or A
             */
            sample = square_lut[lut_index]; // Use the square lookup table for square wave
            break;
        case WAVEFORM_TRIANGLE:
            /* Triangle wave = A*(2/pi)*asin(sin(2*pi*f*t)) = A*(2/pi)*asin(phase)
             * sinf(phase) is in the range [-1, 1], so this will generate a continuous value in the range [-A, A]
             * asin(sin(phase)) is in the range [-pi/2, pi/2], so this will now map the values to [-pi/2, pi/2]
             * (2/pi) is used to scale the value to the range [-A, A]
             */
            sample = triangle_lut[lut_index]; // Use the triangle lookup table for triangle wave
            break;
        case WAVEFORM_INFECTED:
            /* Sawtooth wave = A*(2/pi)*(phase - pi) = A*(2/pi)*(phase)
             * phase is in the range [0, 2*pi], so this will generate a continuous value in the range [-A, A]
             * (2/pi) is used to scale the value to the range [-A, A]
             */
            sample = infected_lut[lut_index]; // Use the infected lookup table for infected wave
            break;
        case WAVEFORM_SWEEP:
            break;
        default:
            break;
        }

        // Write data to the waveform data array
        waveform_data[2 * i] = sample;     // Left channel
        waveform_data[2 * i + 1] = sample; // Right channel

        // Increment phase for the next sample
        phase += phase_inc;

        // Logging will slow down the process, so it is commented out
        // // print package
        // ESP_LOGI(TAG_WAVEFORM, "Sample %d: %d", i, sample);
        // Logging will slow down the process, so it is commented out
    }

}

void initialize_waveform_LUTs()
{
    // Initialize sine lookup table
    for (int i = 0; i < LUT_SIZE; ++i)
    {
        float phase = 2.0f * M_PI * i / LUT_SIZE; // Calculate phase for each index

        sine_lut[i] = (int16_t)(amplitude * sinf(phase));

        square_lut[i] = (i < LUT_SIZE / 2) ? amplitude : -amplitude;

        triangle_lut[i] = (int16_t)(amplitude * (2.0f * i / LUT_SIZE - 1.0f));

        // float infected_phase = 2.0f * M_PI * 150.0f * i / LUT_SIZE; // Calculate phase for infected wave
        // float sine_phase = 2.0f * M_PI * 1500.0f * i / LUT_SIZE; // Calculate phase for sine wave
        // infected_lut[i] = (int16_t)(amplitude * (0.6f * sinf(infected_phase) + 0.4f * sinf(sine_phase)));

        float t = (float)i / (float)LUT_SIZE; // Normalized time from 0 to 1

                // 220Hz "noise" component 
        float infection = 0.6f * sinf(2.0f * M_PI * frequency/2 + 
                      0.3f * sinf(2.0f * M_PI * 5.0f * t));  // 5Hz FM
        
        // 1760Hz main component with harmonic relationship
        float main = 0.4f * sinf(2.0f * M_PI * frequency * t);
        
        // Non-linear mixing with soft clipping
        float infected = tanhf(infection + main);
        
        // Optional: Add slight asymmetry for more character
        if (infected > 0) infected *= 0.9f;
        
        infected_lut[i] = (int16_t)(amplitude * infected);

    }
}
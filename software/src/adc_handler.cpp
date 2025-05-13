#include "adc_handler.h"                                            

#define ADC_CHANNEL ADC2_CHANNEL_3                              // ADC channel for GPIO 15
#define ADC_ATTENUATION ADC_ATTEN_DB_6                          // ADC attenuation, set for 2.2V max input voltage
#define ADC_SAMPLES 64                                          // Number of samples to average
#define ANALOG_READ_PIN 15                                      // GPIO pin for ADC, default is 15
#define ADC_WIDTH ADC_WIDTH_BIT_12                              // ADC width, set to 12 bits

//declarations
static const char *TAG_ADC = "ADC";                             // Tag for logging
static const char* board_id = "Undefined";                             // Board ID based on voltage read from ADC 


//definitions

// Get board ID
const char* get_board_id(void) {
    return board_id;                                            // Return the board ID
}

bool setup_adc(void) {
    bool isSuccesful = false; 
    ESP_LOGI(TAG_ADC, "Setting up ADC...");

    // Sice config function is not boolean, we need to check the return value
    esp_err_t result = adc2_config_channel_atten(ADC_CHANNEL, ADC_ATTENUATION);
    if (result != ESP_OK) {
        ESP_LOGE(TAG_ADC, "Failed to setup ADC! Error code: %s", esp_err_to_name(result));
        isSuccesful = false;
    } else {
        ESP_LOGI(TAG_ADC, "ADC setup successfully!");

        // Set board ID
        identify_and_set_board_id();                            // Call function to identify and set board ID

        isSuccesful = true;
    }

    return isSuccesful;
}

static uint16_t read_adc() {
    uint32_t adc_reading = 0;                                   // Variable to store average ADC reading
    int raw = 0;                                                // Variable to store raw ADC reading, used for averaging  

    // Read ADC samples and average them
    for (int i = 0; i < ADC_SAMPLES; ++i) {
        if (adc2_get_raw(ADC_CHANNEL, ADC_WIDTH, &raw) == ESP_OK) {
            adc_reading += raw;
        }
    }
    adc_reading /= ADC_SAMPLES;                                 // Apply average     

    // Convert to voltage in mV
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_characterize(ADC_UNIT_2, ADC_ATTENUATION, ADC_WIDTH, 0, &adc_chars);

    ESP_LOGI(TAG_ADC, "ADC reading: %d", adc_reading);          // Log the average result of ADC reading
    return esp_adc_cal_raw_to_voltage(adc_reading, &adc_chars); // Return the converted voltage in mV 
}

const char* identify_board(uint16_t voltage_mV) {
    if (voltage_mV < 800){
        board_id = "Board_A";
    }
    else if (voltage_mV < 1600){
        board_id = "Board_B";
    }
    else board_id = "Board_C";

    return board_id;
}

void identify_and_set_board_id(void) {
    uint16_t voltage_mV = read_adc();
    identify_board(voltage_mV);  // Sets global board_id
}

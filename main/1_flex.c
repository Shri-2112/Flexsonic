//For only flex sensor values mapped with audio outputs

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#define SAMPLES 20 // Number of samples for averaging

// Flex Sensor ADC Channels (update to match your wiring)
#define FLEX1 ADC_CHANNEL_0  // GPIO36 (VP) - Thumb
#define FLEX2 ADC_CHANNEL_6  // GPIO34 - Index
#define FLEX3 ADC_CHANNEL_7  // GPIO35 - Middle
#define FLEX4 ADC_CHANNEL_4  // GPIO32 - Ring
#define FLEX5 ADC_CHANNEL_5  // GPIO33 - Pinky

// DFPlayer UART Configuration
#define UART_NUM UART_NUM_1
#define TXD_PIN GPIO_NUM_25   // ESP32 TX to DFPlayer RX (via voltage divider)
#define RXD_PIN GPIO_NUM_26   // ESP32 RX to DFPlayer TX
#define CMD_PLAY_TRACK 0x03

static const char *TAG = "FLEX_DFPLAYER";

adc_oneshot_unit_handle_t adc1_handle;  // ADC Handle

// Send command to DFPlayer Mini
void dfplayer_send_command(uint8_t cmd, uint16_t param) {
    uint8_t packet[10];
    packet[0] = 0x7E;
    packet[1] = 0xFF;
    packet[2] = 0x06;
    packet[3] = cmd;
    packet[4] = 0x00;
    packet[5] = (uint8_t)(param >> 8);
    packet[6] = (uint8_t)(param & 0xFF);
    uint16_t checksum = 0;
    for (int i = 1; i < 7; i++) checksum += packet[i];
    checksum = -checksum;
    packet[7] = (uint8_t)(checksum >> 8);
    packet[8] = (uint8_t)(checksum & 0xFF);
    packet[9] = 0xEF;

    uart_write_bytes(UART_NUM, (const char *)packet, sizeof(packet));
    vTaskDelay(pdMS_TO_TICKS(200));
}

// Play specific MP3 track
void play_mp3_file(int file_number) {
    ESP_LOGI(TAG, "Playing file %04d.mp3", file_number);
    dfplayer_send_command(CMD_PLAY_TRACK, file_number);
}

// Get averaged ADC reading
int get_smoothed_adc_value(int channel) {
    int sum = 0;
    for (int i = 0; i < SAMPLES; i++) {
        int value;
        adc_oneshot_read(adc1_handle, channel, &value);
        sum += value;
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    return sum / SAMPLES;
}

// Check if value is in trigger range
bool is_in_trigger_range(int value) {
    return (value >= 1000 && value <= 3500);
}

void app_main(void) {
    int last_played = 0;

    // ADC Init
    adc_oneshot_unit_init_cfg_t init_config = { .unit_id = ADC_UNIT_1 };
    adc_oneshot_new_unit(&init_config, &adc1_handle);

    adc_oneshot_chan_cfg_t config = { .atten = ADC_ATTEN_DB_11, .bitwidth = ADC_BITWIDTH_12 };
    adc_oneshot_config_channel(adc1_handle, FLEX1, &config);
    adc_oneshot_config_channel(adc1_handle, FLEX2, &config);
    adc_oneshot_config_channel(adc1_handle, FLEX3, &config);
    adc_oneshot_config_channel(adc1_handle, FLEX4, &config);
    adc_oneshot_config_channel(adc1_handle, FLEX5, &config);

    // UART Init
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM, 256, 0, 0, NULL, 0);

    // After UART init
    dfplayer_send_command(0x06, 25); // Set volume to 25/30
    vTaskDelay(pdMS_TO_TICKS(500));

    ESP_LOGI(TAG, "System Ready. Monitoring flex sensors...");
    vTaskDelay(pdMS_TO_TICKS(2000));

    while (1) {
        int thumb   = get_smoothed_adc_value(FLEX1);
        int index   = get_smoothed_adc_value(FLEX2);
        int middle  = get_smoothed_adc_value(FLEX3);
        int ring    = get_smoothed_adc_value(FLEX4);
        int pinky   = get_smoothed_adc_value(FLEX5);

        ESP_LOGI(TAG, "Thumb: %d | Index: %d | Middle: %d | Ring: %d | Pinky: %d",
                 thumb, index, middle, ring, pinky);

        if (is_in_trigger_range(index) && last_played != 1) {
            play_mp3_file(1);
            last_played = 1;
        }
        else if (is_in_trigger_range(thumb) && last_played != 2) {
            play_mp3_file(2);
            last_played = 2; //1000-2500
        }
        else if (is_in_trigger_range(middle) && last_played != 3) {
            play_mp3_file(3);
            last_played = 3; //1000-3500
        }
        else if (is_in_trigger_range(ring) && last_played != 4) {
            play_mp3_file(4);
            last_played = 4;
        }
        else if (is_in_trigger_range(pinky) && last_played != 5) {
            play_mp3_file(5);
            last_played = 5;
        }
        else if (!is_in_trigger_range(thumb) &&
                 !is_in_trigger_range(index) &&
                 !is_in_trigger_range(middle) &&
                 !is_in_trigger_range(ring) &&
                 !is_in_trigger_range(pinky)) {
            last_played = 0; // Reset trigger when no fingers in range
        }

        vTaskDelay(pdMS_TO_TICKS(300));
    }
}





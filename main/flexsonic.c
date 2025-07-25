#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/adc.h"
#include "driver/uart.h"
#include "esp_system.h"
#include "esp_adc/adc_oneshot.h"

#define FLEX_ADC_CHANNEL ADC_CHANNEL_6  // GPIO34
#define UART_TX_PIN 17
#define UART_RX_PIN 16
#define UART_PORT_NUM UART_NUM_2
#define TAG "FLEX_AUDIO"
#define THRESHOLD 100

// Send command to DFPlayer
void sendDFPlayerCommand(uint8_t cmd, uint8_t param) {
    uint8_t packet[10] = {
        0x7E, 0xFF, 0x06, cmd, 0x00, 0x00, 0x00, 0x00, 0xEF
    };
    packet[5] = 0x00;
    packet[6] = param;
    uint16_t checksum = 0 - (packet[1] + packet[2] + packet[3] +
                             packet[4] + packet[5] + packet[6]);
    packet[7] = (checksum >> 8) & 0xFF;
    packet[8] = checksum & 0xFF;
    uart_write_bytes(UART_PORT_NUM, (const char *)packet, 10);
}

void app_main(void) {
    // ADC config
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t adc_unit_cfg = {
        .unit_id = ADC_UNIT_1
    };
    adc_oneshot_new_unit(&adc_unit_cfg, &adc1_handle);
    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_11
    };
    adc_oneshot_config_channel(adc1_handle, FLEX_ADC_CHANNEL, &chan_cfg);

    // UART config
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_driver_install(UART_PORT_NUM, 256, 0, 0, NULL, 0);
    uart_param_config(UART_PORT_NUM, &uart_config);
    uart_set_pin(UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    ESP_LOGI(TAG, "Waiting for DFPlayer to boot...");
    vTaskDelay(pdMS_TO_TICKS(2000)); // Wait 2s for DFPlayer to initialize

    // Set volume to max (48)
    sendDFPlayerCommand(0x06, 0x30); // Set volume to 48
    vTaskDelay(pdMS_TO_TICKS(500));

    bool playing = false;

    while (1) {
        int val;
        adc_oneshot_read(adc1_handle, FLEX_ADC_CHANNEL, &val);
        ESP_LOGI(TAG, "Flex ADC: %d", val);

        if (val > THRESHOLD && !playing) {
            sendDFPlayerCommand(0x0F, 0x01);  // Play track 1 continuously
            ESP_LOGI(TAG, "Playing 0001.mp3");
            playing = true;
        } else if (val <= THRESHOLD && playing) {
            sendDFPlayerCommand(0x16, 0); // Stop playback
            ESP_LOGI(TAG, "Stopping playback");
            playing = false;
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

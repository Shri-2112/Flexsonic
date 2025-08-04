#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_system.h"

#define TXD_PIN (GPIO_NUM_25)
#define RXD_PIN (GPIO_NUM_26)
#define BUF_SIZE (1024)

// ADC channel mapping
#define FLEX1_ADC ADC1_CHANNEL_7  // GPIO35
#define FLEX2_ADC ADC1_CHANNEL_6  // GPIO34
#define FLEX3_ADC ADC1_CHANNEL_5  // GPIO33
#define FLEX4_ADC ADC1_CHANNEL_4  // GPIO32
#define FLEX5_ADC ADC2_CHANNEL_7  // GPIO27

// Value range
#define FLEX_MIN 400
#define FLEX_MAX 500

void send_dfplayer_command(uint8_t cmd, uint16_t param) {
    uint8_t command[10] = {
        0x7E, 0xFF, 0x06, cmd, 0x00,
        (uint8_t)(param >> 8), (uint8_t)(param & 0xFF), 0x00, 0x00, 0xEF
    };

    // Calculate checksum
    uint16_t checksum = 0 - (command[1] + command[2] + command[3] +
                             command[4] + command[5] + command[6]);
    command[7] = (uint8_t)(checksum >> 8);
    command[8] = (uint8_t)(checksum & 0xFF);

    uart_write_bytes(UART_NUM_1, (const char *)command, 10);
}

void init_uart() {
    const uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

void app_main() {
    init_uart();

    // ADC1 init
    adc1_config_width(ADC_WIDTH_BIT_10);
    adc1_config_channel_atten(FLEX1_ADC, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(FLEX2_ADC, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(FLEX3_ADC, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(FLEX4_ADC, ADC_ATTEN_DB_11);

    // ADC2 init for GPIO27
    adc2_config_channel_atten(FLEX5_ADC, ADC_ATTEN_DB_11);

    while (1) {
        int val1 = adc1_get_raw(FLEX1_ADC);
        int val2 = adc1_get_raw(FLEX2_ADC);
        int val3 = adc1_get_raw(FLEX3_ADC);
        int val4 = adc1_get_raw(FLEX4_ADC);
        int val5 = 0;
        adc2_get_raw(FLEX5_ADC, ADC_WIDTH_BIT_10, &val5);

        printf("FLEX1 (GPIO35): %d\n", val1);
        printf("FLEX2 (GPIO34): %d\n", val2);
        printf("FLEX3 (GPIO33): %d\n", val3);
        printf("FLEX4 (GPIO32): %d\n", val4);
        printf("FLEX5 (GPIO27): %d\n", val5);

        // Combination conditions come first so they have higher priority
        if ((val1 >= FLEX_MIN && val1 <= FLEX_MAX) && (val2 >= FLEX_MIN && val2 <= FLEX_MAX)) {
            printf("Combo Track 6 is playing (FLEX1 + FLEX2)...\n");
            send_dfplayer_command(0x03, 6);
            vTaskDelay(3000 / portTICK_PERIOD_MS);
        } else if ((val3 >= FLEX_MIN && val3 <= FLEX_MAX) && (val4 >= FLEX_MIN && val4 <= FLEX_MAX)) {
            printf("Combo Track 7 is playing (FLEX3 + FLEX4)...\n");
            send_dfplayer_command(0x03, 7);
            vTaskDelay(3000 / portTICK_PERIOD_MS);
        } else if ((val2 >= FLEX_MIN && val2 <= FLEX_MAX) && (val5 >= FLEX_MIN && val5 <= FLEX_MAX)) {
            printf("Combo Track 8 is playing (FLEX2 + FLEX5)...\n");
            send_dfplayer_command(0x03, 8);
            vTaskDelay(3000 / portTICK_PERIOD_MS);
        } else if ((val1 >= FLEX_MIN && val1 <= FLEX_MAX) && (val5 >= FLEX_MIN && val5 <= FLEX_MAX)) {
            printf("Combo Track 9 is playing (FLEX1 + FLEX5)...\n");
            send_dfplayer_command(0x03, 9);
            vTaskDelay(3000 / portTICK_PERIOD_MS);
        } else if (val1 >= FLEX_MIN && val1 <= FLEX_MAX) {
            printf("Track 1 is playing...\n");
            send_dfplayer_command(0x03, 1);
            vTaskDelay(3000 / portTICK_PERIOD_MS);
        } else if (val2 >= FLEX_MIN && val2 <= FLEX_MAX) {
            printf("Track 2 is playing...\n");
            send_dfplayer_command(0x03, 2);
            vTaskDelay(3000 / portTICK_PERIOD_MS);
        } else if (val3 >= FLEX_MIN && val3 <= FLEX_MAX) {
            printf("Track 3 is playing...\n");
            send_dfplayer_command(0x03, 3);
            vTaskDelay(3000 / portTICK_PERIOD_MS);
        } else if (val4 >= FLEX_MIN && val4 <= FLEX_MAX) {
            printf("Track 4 is playing...\n");
            send_dfplayer_command(0x03, 4);
            vTaskDelay(3000 / portTICK_PERIOD_MS);
        } else if (val5 >= FLEX_MIN && val5 <= FLEX_MAX) {
            printf("Track 5 is playing...\n");
            send_dfplayer_command(0x03, 5);
            vTaskDelay(3000 / portTICK_PERIOD_MS);
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

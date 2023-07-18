#ifndef __UART_UTILS_H__
#define __UART_UTILS_H__

#include <string.h>
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_event_base.h"

#define EX_UART_NUM UART_NUM_2
#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)
#define BUF_SIZE (256)
#define RD_BUF_SIZE (BUF_SIZE * 2)
#define TX_BUF_SIZE (BUF_SIZE)
#define CMD_BUF_SIZE (64)
#define PATTERN_CHR_NUM (2) /*!< Set the number of consecutive and identical characters received by receiver which defines a UART pattern*/

/**
 * @brief UART Receive Events Base.
 */
ESP_EVENT_DECLARE_BASE(UART_RECEIVE_EVENT);

/**
 * @brief UART Receive Events.
 */
typedef enum
{
    EVT_UART_CUTTING_TAG_REPORT
} uart_receive_event_t;

/** UART Event Subscribe callback prototype
 *
 * @param[in] pvParameters The data passed during subscription
 */
typedef void (*uart_event_subscribe_cb_t)(void *pvParameters);

/**
 * Replace substring in source as new value
 *
 * @note New value should be short than old value
 *
 * @param source
 * @param old_value
 * @param new_value
 *
 */
void replace(char *source, const char *old_value, const char *new_value);

/**
 * Substring
 * @note
 *
 * @param dst
 * @param src
 * @param start
 * @param length
 *
 */
void substr(char *dst, char *src, uint8_t start, uint8_t length);

/**
 * Convert HEX to String
 *
 * @param data
 * @param result    Store HEX string
 * @param length    HEX string length
 *
 */
void hex2str(uint32_t data, char *result, uint8_t length);

/**
 * Calculate check sum of input data.
 *
 * @param data
 * @param result
 *
 */
void calculate_check_sum(char *data, char *result);

/**
 * @brief
 *
 * @param baud_rate
 * @param uart_queue
 * @param uart_event_cb
 */
void uart_init(uint32_t baud_rate, QueueHandle_t *uart_queue, uart_event_subscribe_cb_t uart_event_cb);

#endif /** __UART_UTILS_H__ */
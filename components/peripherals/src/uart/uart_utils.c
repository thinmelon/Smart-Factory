#include "esp_event.h"
#include "uart_utils.h"

ESP_EVENT_DEFINE_BASE(UART_RECEIVE_EVENT);

void uart_init(uint32_t baud_rate, QueueHandle_t *uart_queue, uart_event_subscribe_cb_t uart_event_cb)
{
    uart_config_t uart_config = {
        .baud_rate = baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    // We won't use a buffer for sending data.
    ESP_ERROR_CHECK(uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, uart_queue, 0));
    ESP_ERROR_CHECK(uart_param_config(EX_UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(EX_UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    // Create a task to handler UART event from ISR
    xTaskCreate(uart_event_cb, "uart_event_task", 2048, NULL, configMAX_PRIORITIES, NULL);
}

void replace(char *source, const char *old_value, const char *new_value)
{
    uint32_t length = strlen(source);
    char buffer[length];

    if (0 == length)
        return;

    memset(buffer, 0, sizeof(buffer));
    for (uint8_t i = 0; i < length; i++)
    {
        if (0 == strncmp(source + i, old_value, strlen(old_value)))
        {
            strcat(buffer, new_value);
            i += (strlen(old_value) - 1);
        }
        else
        {
            strncat(buffer, source + i, 1);
        }
    }
    strcpy(source, buffer);
}

void substr(char *dst, char *src, uint8_t start, uint8_t length)
{
    while (start)
    {
        src++;
        start--;
    }

    if (strlen(src) < length)
    {
        length = strlen(src);
    }

    while (length)
    {
        *dst++ = *src++;
        length--;
    }

    *dst = '\0';
}

void hex2str(uint32_t data, char *result, uint8_t length)
{
    int8_t i;
    result[length] = '\0';

    for (i = length - 1; i >= 0; i--, data >>= 4)
    {
        if ((data & 0xf) <= 9)
        {
            result[i] = (data & 0xf) + '0';
        }
        else
        {
            result[i] = (data & 0xf) + 'A' - 0x0a;
        }
    }

    // printf("%d - %s - %d\n", data, result, length);
}

void calculate_check_sum(char *data, char *result)
{
    uint32_t check_sum = 0;
    replace(data, " ", "");
    size_t length = strlen(data);
    char tmp[3];
    for (uint8_t i = 0; i < length; i += 2)
    {
        memset(tmp, 0, 3);
        substr(tmp, data, i, 2);
        check_sum += strtol(tmp, NULL, 16);
    }
    check_sum = check_sum % 256;
    hex2str(check_sum, result, 2);
}

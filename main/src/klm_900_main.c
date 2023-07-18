#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "klm_40505w_m.h"
#include "smart_factory_main.h"

static QueueHandle_t uart2_queue;
static char g_cutting_tag_epc[BUF_SIZE] = {0};
static char g_new_epc_str[BUF_SIZE] = {0};
static const char *TAG = "klm_900_main";

static void uart_event_handler(void *pvParameters)
{
    uart_event_t event;
    uint8_t *raw_data = (uint8_t *)malloc(RD_BUF_SIZE);

    for (;;)
    {
        // Waiting for UART event.
        if (xQueueReceive(uart2_queue, (void *)&event, (portTickType)portMAX_DELAY))
        {
            bzero(raw_data, RD_BUF_SIZE);
            switch (event.type)
            {
            // Event of UART receving data
            /* We'd better handler data event fast, there would be much more data events than
            other types of events. If we take too much time on data event, the queue might
            be full. */
            case UART_DATA:
                uart_read_bytes(EX_UART_NUM, raw_data, event.size, portMAX_DELAY);
                ESP_LOG_BUFFER_HEXDUMP("[UART DATA]: ", raw_data, event.size, ESP_LOG_INFO);
                klm_read_bytes(raw_data, g_new_epc_str);
                break;
            // Event of HW FIFO overflow detected
            case UART_FIFO_OVF:
                ESP_LOGI(TAG, "hw fifo overflow");
                // If fifo overflow happened, you should consider adding flow control for your application.
                // The ISR has already reset the rx FIFO,
                // As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(EX_UART_NUM);
                xQueueReset(uart2_queue);
                break;
            // Event of UART ring buffer full
            case UART_BUFFER_FULL:
                ESP_LOGI(TAG, "ring buffer full");
                // If buffer full happened, you should consider encreasing your buffer size
                // As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(EX_UART_NUM);
                xQueueReset(uart2_queue);
                break;
            // Event of UART RX break detected
            case UART_BREAK:
                ESP_LOGI(TAG, "uart rx break");
                break;
            // Event of UART parity check error
            case UART_PARITY_ERR:
                ESP_LOGI(TAG, "uart parity error");
                break;
            // Event of UART frame error
            case UART_FRAME_ERR:
                ESP_LOGI(TAG, "uart frame error");
                break;
            // UART_PATTERN_DET
            case UART_PATTERN_DET:
                break;
            // Others
            default:
                ESP_LOGI(TAG, "uart event type: %d", event.type);
                break;
            }
        }
    }
    free(raw_data);
    raw_data = NULL;
    vTaskDelete(NULL);
}

static void klm_900_tx_task(void *arg)
{
    static const char *TX_TASK_TAG = "KLM900_TX_TASK";
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);

    while (1)
    {
        //  Send reading multi-tag command
        ESP_ERROR_CHECK(klm_read_multi_tag_id(32768));
        klm_write_bytes();
        vTaskDelay(3000 / portTICK_PERIOD_MS);

        //  Stop reading multi-tag command
        ESP_ERROR_CHECK(klm_stop_read_multi_tag_id());
        klm_write_bytes();
        vTaskDelay(2000 / portTICK_PERIOD_MS);

        //  Send select command
        ESP_ERROR_CHECK(klm_set_select(0x01, 0x00, 0x00, 0x00000004, 0x60, g_new_epc_str, 0x00));
        klm_write_bytes();

        //  Send LED on command, if available
        ESP_ERROR_CHECK(klm_read_tag_data("00000000", 0x00, 0x0004, 0x0001));
        klm_write_bytes();
        
        // Compare new value with old one, copy the new one and sync with qcloud if changed.
        if ((strlen(g_new_epc_str) > 0) &&
            (0 != strcmp(g_cutting_tag_epc, g_new_epc_str)))
        {
            bzero(g_cutting_tag_epc, BUF_SIZE);
            strcpy(g_cutting_tag_epc, g_new_epc_str);
            esp_event_post(DEVICE_EVENT, EVT_DEVICE_REPORT_PROPERTY, NULL, 0, portMAX_DELAY);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

char *klm_900_get_cutting_tag_epc(void)
{
    return g_cutting_tag_epc;
}

void klm_900_set_cutting_tag_epc(const char* epc_str)
{
    bzero(g_cutting_tag_epc, BUF_SIZE);
    strcpy(g_cutting_tag_epc, epc_str);
}

void klm_900_init(void)
{
    uart_init(115200, &uart2_queue, uart_event_handler);
    xTaskCreate(klm_900_tx_task, "klm_900_tx_task", 1024 * 2, NULL, configMAX_PRIORITIES - 1, NULL);
}

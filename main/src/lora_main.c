#include <stdio.h>
#include "esp_qcloud_storage.h"
#include "lora.h"
#include "smart_factory_main.h"

static const char *TAG = "lora_main";
static device_status_t g_device_status = {0};
static uint8_t buf[32];

static void task_rx(void *p)
{

    int x;
    for (;;)
    {
        lora_receive(); // put into receive mode
        while (lora_received())
        {
            x = lora_receive_packet(buf, sizeof(buf));
            buf[x] = 0;
            ESP_LOGI(TAG, "Received message: %s", buf);
            lora_receive();
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* Event handler for catching button events */
static void device_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case EVT_DEVICE_REPORT_PROPERTY:
        ESP_LOGI(TAG, "=== EVT_DEVICE_REPORT_PROPERTY ===");

        cJSON *params_json = cJSON_CreateObject();

        ESP_ERROR_CHECK(esp_qcloud_storage_get(DEVICE_STATUS_NVS_KEY, &g_device_status, sizeof(g_device_status)));
        ESP_LOGI(TAG, "device_event_handler - [%s]: %d", "PROPERTY_CUT_NUMBER", g_device_status.counting);
        cJSON_AddNumberToObject(params_json, "PROPERTY_CUT_NUMBER", g_device_status.counting);
        char *params_json_string = cJSON_Print(params_json);
        ESP_LOGI(TAG, "device_event_handler - %s", params_json_string);
        lora_send_packet((uint8_t *)params_json_string, strlen(params_json_string));

        cJSON_Delete(params_json);

        break;

    default:
        ESP_LOGW(TAG, "Unhandled Device Trigger Event: %d", event_id);
    }
}

void lora_slave_run(void)
{
    lora_init();
    lora_set_frequency(RF_FREQUENCY);
    lora_set_tx_config(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                       LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                       LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                       pdTRUE, 0, 0, LORA_IQ_INVERSION_ON, 3000);
    lora_enable_crc();

    esp_event_loop_create_default();
    ESP_ERROR_CHECK(esp_event_handler_instance_register(DEVICE_EVENT, ESP_EVENT_ANY_ID, device_event_handler, NULL, NULL));
}

void lora_master_run(void)
{
    lora_init();
    lora_set_frequency(RF_FREQUENCY);
    lora_set_rx_config(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                       LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                       LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                       0, pdTRUE, 0, 0, LORA_IQ_INVERSION_ON, pdTRUE);
    lora_enable_crc();
    xTaskCreate(&task_rx, "task_rx", 2048, NULL, 5, NULL);
}
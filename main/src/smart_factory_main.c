#include <stdio.h>

#include "driver/gpio.h"
#include "esp_event.h"
#include "esp_qcloud_log.h"
#include "esp_qcloud_storage.h"

#include "lora.h"
#include "button.h"
#include "smart_factory_main.h"

static const char *TAG = "smart_factory_main";
static device_status_t g_device_status = {0};

ESP_EVENT_DEFINE_BASE(DEVICE_EVENT);

/**
 * @brief Save the counting number into NVS flash.
 *
 * @note When a reset message is received, set the counting number as zero.
 * Otherwise, auto increase the counting number which is global value.
 *
 * @param[in] value
 * @param[in] auto_increase
 * @return
 *     - ESP_OK: succeed
 *     - others: fail
 */
static esp_err_t set_device_status(uint32_t value, uint8_t auto_increase)
{
    g_device_status.counting = auto_increase ? g_device_status.counting + 1 : value;
    esp_qcloud_storage_set(DEVICE_STATUS_NVS_KEY, &g_device_status, sizeof(g_device_status));
    // oled_show_number(0, 3, g_device_status.counting, 10, 16);

    return ESP_OK;
}

/**
 * @brief Callback to handle button event.
 *
 * @note .
 *
 * @param[in] params
 */
static void button_event_handler(void *params)
{
    if (!strcmp((char *)params, "RESET"))
    {
        ESP_LOGI(TAG, "Reset...");
        set_device_status(0, 0);
        esp_event_post(DEVICE_EVENT, EVT_DEVICE_REPORT_PROPERTY, NULL, 0, portMAX_DELAY);
    }
    else if (!strcmp((char *)params, "COUNT"))
    {
        ESP_LOGI(TAG, "Counting...");
        set_device_status(0, 1);
        esp_event_post(DEVICE_EVENT, EVT_DEVICE_REPORT_PROPERTY, NULL, 0, portMAX_DELAY);
    }
    else
    {
        ESP_LOGW(TAG, "This parameter is not supported");
    }
}

#ifdef CONFIG_NETWORK_USE_WIFI

/**
 * @brief Callback to handle commands send to the QCloud cloud.
 *
 * @note .
 *
 * @param[in] id
 * @param[out] val
 * @return
 *     - ESP_OK: succeed
 *     - others: fail
 */
static esp_err_t device_get_param_handler(const char *id, esp_qcloud_param_val_t *val)
{
    if (!strcmp(id, "PROPERTY_CUT_NUMBER"))
    {
        esp_qcloud_storage_get(DEVICE_STATUS_NVS_KEY, &g_device_status, sizeof(g_device_status));
        val->i = g_device_status.counting;
        ESP_LOGI(TAG, "device_get_param_handler - [%s]: %d", id, val->i);
    }

    // if (!strcmp(id, "PROPERTY_WIFI_MAC"))
    // {
    //     val->s = g_mac_str;
    //     ESP_LOGI(TAG, "device_get_param_handler - [%s]: %s", id, val->s);
    // }

    return ESP_OK;
}

/**
 * @brief Callback to handle commands received from the QCloud cloud.
 *
 * @note .
 *
 * @param[in] id
 * @param[in] val
 * @return
 *     - ESP_OK: succeed
 *     - others: fail
 */
static esp_err_t device_set_param_handler(const char *id, const esp_qcloud_param_val_t *val)
{
    esp_err_t err = ESP_FAIL;
    ESP_LOGI(TAG, "Received id: %s, val: %d", id, val->i);

    if (!strcmp(id, "PROPERTY_CUT_NUMBER"))
    {
        set_device_status(val->i, 0);
        err = ESP_OK;
    }
    else
    {
        ESP_LOGW(TAG, "This parameter is not supported");
    }

    return err;
}

/**
 * @brief Callback to handle actions received from the QCloud cloud.
 *
 * @note .
 *
 * @param[in] action_handle
 * @param[in] params
 * @return
 *     - ESP_OK: succeed
 *     - others: fail
 */
static esp_err_t device_set_action_handler(esp_qcloud_method_t *action_handle, char *params)
{
    ESP_LOGI(TAG, "  device_set_action_handler  Received action: %s", params);

    esp_err_t err = ESP_OK;
    cJSON *params_data = cJSON_Parse(params);
    cJSON *params_action_id = cJSON_GetObjectItem(params_data, "actionId");
    cJSON *params_token = cJSON_GetObjectItem(params_data, "token");

    if (!strcmp(params_action_id->valuestring, "ACTION_RESET_CUT_NUMBER"))
    {
        set_device_status(0, 0);
        esp_event_post(DEVICE_EVENT, EVT_DEVICE_REPORT_PROPERTY, NULL, 0, portMAX_DELAY);
    }

    esp_qcloud_iothub_param_add_string(action_handle, "result", err == ESP_OK ? "Success" : "Fail");
    esp_qcloud_iothub_param_add_string(action_handle, "token", params_token->valuestring);

    cJSON_Delete(params_data);
    return err;
}
#endif

void app_main(void)
{
    /**
     * @brief LED Configuration
     */
    gpio_config_t blink = {
        .pin_bit_mask = BIT64(CONFIG_GPIO_LED_GREEN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = 0,
        .pull_up_en = 0};
    ESP_ERROR_CHECK(gpio_config(&blink));

    /**
     * @brief NVS
     *
     * @note 4354 means the key was not found on the NVS, so reset counting number as 0 then save it into NVS flash.
     */
    esp_qcloud_storage_init();
    if (4354 == esp_qcloud_storage_get(DEVICE_STATUS_NVS_KEY, &g_device_status, sizeof(g_device_status)))
    {
        ESP_LOGI(TAG, "<ESP_ERR_NVS_NOT_FOUND> Reset counting number.");
        g_device_status.counting = 0;
        esp_qcloud_storage_set(DEVICE_STATUS_NVS_KEY, &g_device_status, sizeof(g_device_status));
    }
    else
    {
        ESP_LOGI(TAG, "<esp_qcloud_storage_get> Current counting number: %d", g_device_status.counting);
    }

#ifdef CONFIG_LORA_ROLE_SLAVE
    /**
     * @brief Lora Slave Configuration
     *
     * @note SX1278
     */
    lora_slave_run();
    // xTaskCreate(&task_tx, "task_tx", 2048, NULL, 5, NULL);

    /**
     * @brief BUTTON
     * Configure the button GPIO as input
     */
    button_handle_t btn_handle = button_create(CONFIG_GPIO_DEVICE_RESET, BUTTON_ACTIVE_LOW);
    button_set_evt_cb(btn_handle, BUTTON_CB_TAP, button_event_handler, "RESET");
    btn_handle = button_create(CONFIG_GPIO_DEVICE_COUNTING, BUTTON_ACTIVE_LOW);
    button_set_evt_cb(btn_handle, BUTTON_CB_TAP, button_event_handler, "COUNT");
#endif

#ifdef CONFIG_LORA_ROLE_MASTER
    /**
     * @brief Lora Master Configuration
     *
     * @note SX1278
     */
    lora_master_run();

#endif

#ifdef CONFIG_NETWORK_USE_WIFI
    /**
     * @brief BUTTON
     * Configure the button GPIO as input
     */
    button_handle_t btn_handle = button_create(CONFIG_GPIO_DEVICE_RESET, BUTTON_ACTIVE_LOW);
    button_set_evt_cb(btn_handle, BUTTON_CB_TAP, button_event_handler, "RESET");
    btn_handle = button_create(CONFIG_GPIO_DEVICE_COUNTING, BUTTON_ACTIVE_LOW);
    button_set_evt_cb(btn_handle, BUTTON_CB_TAP, button_event_handler, "COUNT");

    /**
     * @brief Tencent QCloud IotHub
     *
     */
    esp_qcloud_init(device_get_param_handler,
                    device_set_param_handler,
                    device_set_action_handler);
#endif
}

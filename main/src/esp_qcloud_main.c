#include "esp_qcloud_log.h"
#include "esp_qcloud_console.h"
#include "esp_qcloud_mqtt.h"
#include "esp_qcloud_storage.h"
#include "esp_qcloud_prov.h"

#ifdef CONFIG_BT_ENABLE
#include "esp_bt.h"
#endif

#include "smart_factory_main.h"

static const char *TAG = "esp_qcloud_main";
static bool g_qcloud_mqtt_is_connected = false;

/* Event handler for catching QCloud events */
static void qcloud_event_handler(void *arg, esp_event_base_t event_base,
                                 int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case QCLOUD_EVENT_IOTHUB_INIT_DONE:
        esp_qcloud_iothub_report_device_info();
        ESP_LOGI(TAG, "QCloud Initialised");
        break;

    case QCLOUD_EVENT_IOTHUB_BOND_DEVICE:
        ESP_LOGI(TAG, "Device binding successful");
        break;

    case QCLOUD_EVENT_IOTHUB_UNBOND_DEVICE:
        ESP_LOGW(TAG, "Device unbound with iothub");
        esp_qcloud_wifi_reset();
        esp_restart();
        break;

    case QCLOUD_EVENT_IOTHUB_BIND_EXCEPTION:
        ESP_LOGW(TAG, "Device bind fail");
        esp_qcloud_wifi_reset();
        esp_restart();
        break;

    case QCLOUD_EVENT_IOTHUB_RECEIVE_STATUS:
        ESP_LOGI(TAG, "receive status message: %s", (char *)event_data);
        break;

    default:
        ESP_LOGW(TAG, "Unhandled QCloud Event: %d", event_id);
    }
}

/* Event handler for catching button events */
static void device_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case EVT_DEVICE_REPORT_PROPERTY:
        ESP_LOGI(TAG, "=== EVT_DEVICE_REPORT_PROPERTY === %d", g_qcloud_mqtt_is_connected);
        if (g_qcloud_mqtt_is_connected)
        {
            /* Report all property when received device's trigger event */
            ESP_ERROR_CHECK(esp_qcloud_iothub_report_all_property());
        }
        break;

    default:
        ESP_LOGW(TAG, "Unhandled Device Trigger Event: %d", event_id);
    }
}

/* Event handler for catching mqtt events */
static void mqtt_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case QCLOUD_MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "=== QCLOUD_MQTT_EVENT_CONNECTED ===");
        g_qcloud_mqtt_is_connected = true;
        /* Report all property after reconnection */
        ESP_ERROR_CHECK(esp_qcloud_iothub_report_all_property());
        break;

    case QCLOUD_MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "=== QCLOUD_MQTT_EVENT_DISCONNECTED ===");
        g_qcloud_mqtt_is_connected = false;
        break;

    default:
        ESP_LOGW(TAG, "Unhandled QCloud MQTT Event: %d", event_id);
    }
}

static esp_err_t get_wifi_config(wifi_config_t *wifi_cfg, uint32_t wait_ms)
{
    ESP_QCLOUD_PARAM_CHECK(wifi_cfg);

    if (esp_qcloud_storage_get("wifi_config", wifi_cfg, sizeof(wifi_config_t)) == ESP_OK)
    {

#ifdef CONFIG_BT_ENABLE
        esp_bt_controller_mem_release(ESP_BT_MODE_BTDM);
#endif

        return ESP_OK;
    }

#ifdef CONFIG_NETWORK_WIFI_AUTH_OPEN
    memcpy(wifi_cfg->sta.ssid, CONFIG_NETWORK_WIFI_AUTH_OPEN_SSID, strlen(CONFIG_NETWORK_WIFI_AUTH_OPEN_SSID));
    wifi_cfg->sta.threshold.authmode = WIFI_AUTH_OPEN;
    wifi_cfg->sta.pmf_cfg.capable = true;
    wifi_cfg->sta.pmf_cfg.required = false;

    return ESP_OK;
#endif

    /**< Reset wifi and restart wifi */
    esp_wifi_restore();
    esp_wifi_start();

    /**< The yellow light flashes to indicate that the device enters the state of configuring the network */
    // light_driver_breath_start(128, 128, 0); /**< yellow blink */

    /**< Note: Smartconfig and softapconfig working at the same time will affect the configure network performance */

#ifdef CONFIG_NETWORK_PROVISIONING_SOFTAPCONFIG
    char softap_ssid[32 + 1] = CONFIG_NETWORK_PROVISIONING_SOFTAPCONFIG_SSID;
    // uint8_t mac[6] = {0};
    // ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_STA, mac));
    // sprintf(softap_ssid, "tcloud_%s_%02x%02x", light_driver_get_type(), mac[4], mac[5]);

    esp_qcloud_prov_softapconfig_start(SOFTAPCONFIG_TYPE_ESPRESSIF_TENCENT,
                                       softap_ssid,
                                       CONFIG_NETWORK_PROVISIONING_SOFTAPCONFIG_PASSWORD);
    esp_qcloud_prov_print_wechat_qr(softap_ssid, "softap");
#endif

#ifdef CONFIG_NETWORK_PROVISIONING_SMARTCONFIG
    esp_qcloud_prov_smartconfig_start(SC_TYPE_ESPTOUCH_AIRKISS);
#endif

#ifdef CONFIG_NETWORK_PROVISIONING_BLECONFIG
    char local_name[32 + 1] = CONFIG_NETWORK_PROVISIONING_BLECONFIG_NAME;
    esp_qcloud_prov_bleconfig_start(BLECONFIG_TYPE_ESPRESSIF_TENCENT, local_name);
#endif

    ESP_ERROR_CHECK(esp_qcloud_prov_wait(wifi_cfg, wait_ms));

#ifdef CONFIG_NETWORK_PROVISIONING_SMARTCONFIG
    esp_qcloud_prov_smartconfig_stop();
#endif

#ifdef CONFIG_NETWORK_PROVISIONING_SOFTAPCONFIG
    esp_qcloud_prov_softapconfig_stop();
#endif

    /**< Store the configure of the device */
    esp_qcloud_storage_set("wifi_config", wifi_cfg, sizeof(wifi_config_t));

    /**< Configure the network successfully to stop the light flashing */
    // light_driver_breath_stop(); /**< stop blink */

    return ESP_OK;
}

bool esp_qcloud_is_mqtt_connected(void)
{
    return g_qcloud_mqtt_is_connected;
}

void esp_qcloud_init(const esp_qcloud_get_param_t get_param_cb,
                     const esp_qcloud_set_param_t set_param_cb,
                     const esp_qcloud_action_cb_t action_cb)
{
    /**
     * @brief Add debug function, you can use serial command and remote debugging.
     */
    // esp_qcloud_log_config_t log_config = {
    //     .log_level_uart = ESP_LOG_INFO,
    // };
    // ESP_ERROR_CHECK(esp_qcloud_log_init(&log_config));
    /**
     * @brief Set log level
     * @note  This function can not raise log level above the level set using
     * CONFIG_LOG_DEFAULT_LEVEL setting in menuconfig.
     */
    // esp_log_level_set("*", ESP_LOG_VERBOSE);
    /**
     * @brief Initialize Application specific hardware drivers and set initial state.
     */
    // light_driver_config_t driver_cfg = COFNIG_LIGHT_TYPE_DEFAULT();
    // ESP_ERROR_CHECK(light_driver_init(&driver_cfg));

    /**< Continuous power off and restart more than five times to reset the device */

    //     if (esp_qcloud_reboot_unbroken_count() >= CONFIG_LIGHT_REBOOT_UNBROKEN_COUNT_RESET) {
    //         ESP_LOGW(TAG, "Erase information saved in flash");
    //         esp_qcloud_storage_erase(CONFIG_QCLOUD_NVS_NAMESPACE);
    //     } else if (esp_qcloud_reboot_is_exception(false)) {
    //         ESP_LOGE(TAG, "The device has been restarted abnormally");
    // #ifdef CONFIG_LIGHT_DEBUG
    //         light_driver_breath_start(255, 0, 0); /**< red blink */
    // #else
    //         ESP_ERROR_CHECK(light_driver_set_switch(true));
    // #endif /**< CONFIG_LIGHT_DEBUG */
    //     } else {
    //         ESP_ERROR_CHECK(light_driver_set_switch(true));
    //     }

    /*
     * @breif Create a device through the server and obtain configuration parameters
     *        server: https://console.cloud.tencent.com/iotexplorer
     */
    /**< Create and configure device authentication information */
    ESP_ERROR_CHECK(esp_qcloud_create_device());
    /**< Configure the version of the device, and use this information to determine whether to OTA */
    // ESP_ERROR_CHECK(esp_qcloud_device_add_fw_version("0.0.1"));
    /**< Register the properties of the device */
    ESP_ERROR_CHECK(esp_qcloud_device_add_property("PROPERTY_CUT_NUMBER", QCLOUD_VAL_TYPE_INTEGER));
    ESP_ERROR_CHECK(esp_qcloud_device_add_property("PROPERTY_CUT_RFID_EPC", QCLOUD_VAL_TYPE_STRING));
    /**< The processing function of the communication between the device and the server */
    ESP_ERROR_CHECK(esp_qcloud_device_add_property_cb(get_param_cb, set_param_cb));
    ESP_ERROR_CHECK(esp_qcloud_device_add_action_cb("ACTION_RESET_CUT_NUMBER", action_cb));

    /**
     * @brief Initialize Wi-Fi.
     */
    ESP_ERROR_CHECK(esp_qcloud_wifi_init());
    ESP_ERROR_CHECK(esp_event_handler_instance_register(QCLOUD_EVENT, ESP_EVENT_ANY_ID, &qcloud_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(DEVICE_EVENT, ESP_EVENT_ANY_ID, device_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(QCLOUD_MQTT_EVENT, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL, NULL));

    /**
     * @brief Get the router configuration
     */
    wifi_config_t wifi_cfg = {0};
    ESP_ERROR_CHECK(get_wifi_config(&wifi_cfg, portMAX_DELAY));

    /**
     * @brief Connect to router
     */
    ESP_ERROR_CHECK(esp_qcloud_wifi_start(&wifi_cfg));
    ESP_ERROR_CHECK(esp_qcloud_timesync_start());
    esp_qcloud_timesync_wait(1000);

#ifdef CONFIG_AUTH_MODE_DYNREG
    /**
     * @brief Dynamic register device
     */
    ESP_ERROR_CHECK(esp_qcloud_dynreg_device());
#endif

    /**
     * @brief Connect to Tencent Cloud Iothub
     */
    ESP_ERROR_CHECK(esp_qcloud_iothub_init());
    ESP_ERROR_CHECK(esp_qcloud_iothub_start());
    ESP_ERROR_CHECK(esp_qcloud_iothub_ota_enable());
}
#ifndef __SMART_FACTORY_H__
#define __SMART_FACTORY_H__

#include <stdio.h>
#include <string.h>

#include "esp_event_base.h"
#include "esp_qcloud_iothub.h"
#include "uart_utils.h"

#define DEVICE_STATUS_NVS_KEY "device_status"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Device Trigger Event Base.
     */
    ESP_EVENT_DECLARE_BASE(DEVICE_EVENT);

    /**
     * @brief Device Trigger Events.
     */
    typedef enum
    {
        EVT_DEVICE_REPORT_PROPERTY = 11
    } device_event_t;

    /**
     * @brief Device status struct which stores in the NVS flash.
     */
    typedef struct
    {
        uint32_t counting;
        char tag_epc[BUF_SIZE];
    } device_status_t;

    /**
     * @brief Config tencent cloud service
     *
     * @note .
     *
     * @param[in] get_param_cb sync property with qcloud callback
     * @param[in] set_param_cb set property by command from qcloud callback
     * @param[in] action_cb receive action from qcloud
     */
    void esp_qcloud_init(const esp_qcloud_get_param_t get_param_cb,
                         const esp_qcloud_set_param_t set_param_cb,
                         const esp_qcloud_action_cb_t action_cb);

    /**
     * @brief Check MQTT status
     * 
     * @note Provide API for outter files to access MQTT status which is stored in global variable
     * 
     * @return ture or false
     */
    bool esp_qcloud_is_mqtt_connected(void);

    /**
     * @brief KLM900 Get RFID TAG's EPC
     * 
     * @note Provide API for outter files to access EPC which is stored in global variable
     * 
     * @return EPC
     */
    char *klm_900_get_cutting_tag_epc(void);

    /**
     * @brief KLM900 Set RFID TAG's EPC
     * 
     * @note Provide API to initialize RFID TAG's EPC 
     * 
     * @param[in] epc_str EPC string
     */
    void klm_900_set_cutting_tag_epc(const char* epc_str);

    /**
     * @brief Initialize KLM 900 module
     * 
     * @note Start a task to communicate with KLM module
     */
    void klm_900_init(void);

    /**
     * @brief Initialize FM 507 module
     * 
     */
    void fm_507_init(void);

#ifdef __cplusplus
}
#endif

#endif
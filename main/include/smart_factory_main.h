#ifndef __SMART_FACTORY_H__
#define __SMART_FACTORY_H__

#include <stdio.h>
#include <string.h>

#include "esp_event_base.h"
#include "esp_qcloud_iothub.h"

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
    } button_event_t;

    /**
     * @brief Device status struct which stores in the NVS flash.
     */
    typedef struct
    {
        uint32_t counting;
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
     * @brief Config LoRa task
     *
     * @note Two roles: master and slave.
     *
     */
    void lora_slave_run(void);

    /**
     * @brief Config LoRa task
     *
     * @note Two roles: master and slave.
     *
     */
    void lora_master_run(void);

#ifdef __cplusplus
}
#endif

#endif
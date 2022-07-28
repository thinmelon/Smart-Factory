// Copyright 2020 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string.h>

#include "utils_aes.h"
#include "utils_base64.h"
#include "utils_hmac.h"
#include "esp_http_client.h"

#include "esp_qcloud_iothub.h"

static const char *TAG = "esp_qcloud_dynreg";
static char g_http_client_response_buffer[DYN_REG_RSP_BUFF_LEN] = {0};
static uint32_t g_seed = 1;

/**
 * @brief Generate signature for dynamic register device.
 *
 * @param[in] device_profile device info.
 * @param[in] signout signature
 * @param[in] max_signlen signature's length
 * @param[in] nonce nonce
 * @param[in] timestamp timestamp
 * @return esp_err_t
 */
static esp_err_t esp_qcloud_dynreg_generate_signature(esp_qcloud_profile_t *device_profile, char *signout, uint8_t max_signlen, uint32_t nonce, time_t timestamp)
{
    uint32_t sign_len;
    size_t olen = 0;
    char *sign_source_ptr = NULL;
    const char *sign_fmt = "deviceName=%s&nonce=%d&productId=%s&timestamp=%d";
    char sign[DYN_REG_SIGN_LEN] = {0};

    sign_len = strlen(sign_fmt) + strlen(device_profile->device_name) + strlen(device_profile->product_id) + sizeof(uint32_t) +
               sizeof(time_t) + DYN_BUFF_DATA_MORE;
    sign_source_ptr = ESP_QCLOUD_MALLOC(sign_len);
    if (sign_source_ptr == NULL)
    {
        ESP_LOGE(TAG, "Malloc memory for signature source buffer failed!!");
        return ESP_FAIL;
    }
    memset(sign_source_ptr, 0, sign_len);
    snprintf((char *)sign_source_ptr, sign_len, sign_fmt, device_profile->device_name, nonce, device_profile->product_id,
             timestamp);
    utils_hmac_sha1(sign_source_ptr, strlen(sign_source_ptr), sign, device_profile->product_secret, strlen(device_profile->product_secret));
    utils_base64encode((uint8_t *)signout, max_signlen, &olen, (const uint8_t *)sign, strlen(sign));

    ESP_QCLOUD_FREE(sign_source_ptr);

    return (olen > max_signlen) ? ESP_FAIL : ESP_OK;
}

static uint32_t get_time_ms(void)
{
    struct timeval time_val = {0};
    uint32_t time_ms;

    gettimeofday(&time_val, NULL);
    time_ms = time_val.tv_sec * 1000 + time_val.tv_usec / 1000;

    return time_ms;
}

static uint32_t rand_d(void)
{
    return rand_r(&g_seed);
}

static void srand_d(uint32_t i)
{
    g_seed = i;
}

/**
 * @brief Parse data to fetch payload after recive response from tencent dynamic register server.
 *        Decrypt payload to get psk.
 * 
 * @note Response general format example
 *  {"code":0,"message":"success","len":53,"payload":"JdzLo0vRERuOqX9vJCRnsfhax/Q14ZR8iWxCf8+fLpTmMQPKiOZip9SPf3tkHNYGYKzPexIisziieEDpamZtCw==","requestId":"007181eb-e94f-4dfe-b0ff-c1d239b48d97"}
 * 
 * @param[in] device_profile device info.
 * @param[in] raw_data http response
 * @return esp_err_t
 */
static void esp_qcloud_dynreg_parse_response_payload(esp_qcloud_profile_t *device_profile, const char *raw_data)
{
    int ret = 0;
    char decodeBuffer[DECODE_BUFF_LEN] = {0};
    int datalen;
    unsigned int keybits;
    char key[PRODUCT_SECRET_MAX_SIZE + 1];
    unsigned char iv[16];

    cJSON *json_data = cJSON_Parse(raw_data);
    cJSON *code = cJSON_GetObjectItem(json_data, "code");
    cJSON *message = cJSON_GetObjectItem(json_data, "message");
    cJSON *len = cJSON_GetObjectItem(json_data, "len");
    cJSON *payload = cJSON_GetObjectItem(json_data, "payload");
    cJSON *decrypted_data = NULL;
    size_t length = len->valueint;

    if (ESP_OK != code->valueint)
    {
        ESP_LOGE(TAG, "Response dynamic register device failed, code: %d, message: %s", code->valueint, message->valuestring);
        goto EXIT;
    }
    else
    {
        if (payload->valuestring == NULL)
        {
            ESP_LOGE(TAG, "Invalid json content: %s", raw_data);
            goto EXIT;
        }
    }

    ret = utils_base64decode((uint8_t *)decodeBuffer, sizeof(decodeBuffer), &length, (uint8_t *)payload->valuestring,
                             strlen(payload->valuestring));
    if (ESP_OK != ret)
    {
        ESP_LOGE(TAG, "Decode failed with return %d.", ret);
        goto EXIT;
    }

    datalen = length + (UTILS_AES_BLOCK_LEN - length % UTILS_AES_BLOCK_LEN);
    keybits = AES_KEY_BITS_128;
    memset(key, 0, PRODUCT_SECRET_MAX_SIZE);
    strncpy(key, device_profile->product_secret, PRODUCT_SECRET_MAX_SIZE);
    memset(iv, '0', UTILS_AES_BLOCK_LEN);
    ret = utils_aes_cbc((uint8_t *)decodeBuffer, datalen, (uint8_t *)decodeBuffer, DECODE_BUFF_LEN, UTILS_AES_DECRYPT,
                        (uint8_t *)key, keybits, iv);
    if (ESP_OK != ret)
    {
        ESP_LOGE(TAG, "Data decrypt failed,ret:%d", ret);
        goto EXIT;
    }

    decrypted_data = cJSON_Parse(decodeBuffer);
    cJSON *encryption_type = cJSON_GetObjectItem(decrypted_data, "encryptionType");
    cJSON *psk = cJSON_GetObjectItem(decrypted_data, "psk");

#ifdef CONFIG_AUTH_MODE_CERT

#else
    if (QCLOUD_AUTH_MODE_KEY + 1 != encryption_type->valueint)
    {
        ESP_LOGE(TAG, "Encryt type should be psk type!!");
        goto EXIT;
    }

    if (NULL != psk->valuestring)
    {
        if (strlen(psk->valuestring) > DEVICE_SECRET_SIZE)
        {
            ESP_LOGE(TAG, "PSK exceed max length, %s", psk->valuestring);
        }
        else
        {
            esp_qcloud_device_secret(psk->valuestring);
        }
    }
    else
    {
        ESP_LOGE(TAG, "Get PSK data failed!");
    }
#endif

EXIT:
    if (json_data != NULL)
    {
        cJSON_Delete(json_data);
    }
    if (decrypted_data != NULL)
    {
        cJSON_Delete(decrypted_data);
    }
}

/**
 * @brief Http event handler which passed to esp_http_client_config_t.
 *        You need to response all the events.
 * 
 * @param[in] evt http client event parameter
 * @return esp_err_t
 */
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        break;
    case HTTP_EVENT_ON_CONNECTED:
        break;
    case HTTP_EVENT_HEADER_SENT:
        break;
    case HTTP_EVENT_ON_HEADER:
        break;
    case HTTP_EVENT_ON_DATA:
        // ESP_LOGI(TAG, ">>>>>>>>>>  HTTP_EVENT_ON_DATA  <<<<<<<<<<<");
        if (!esp_http_client_is_chunked_response(evt->client))
        {
            // ESP_LOGI(TAG, "%.*s", evt->data_len, (char *)evt->data);
            strncpy((char *)g_http_client_response_buffer, (const char *)evt->data, evt->data_len);
        }
        break;
    case HTTP_EVENT_ON_FINISH:
        break;
    case HTTP_EVENT_DISCONNECTED:
        break;
    }
    
    return ESP_OK;
}

/**
 * @brief Send http post request to tencent dynamic register server.
 *        Constuct post data with necessary params required by tencent. 
 * 
 * 
 * @param[in] device_profile device profile
 * @param[in] url_string request url
 * @param[in] request_json_str post data using json format
 * @return esp_err_t
 */
static esp_err_t _http_rest_with_url(esp_qcloud_profile_t *device_profile, const char *url_string, const char *request_json_str)
{
    // char local_response_buffer[256] = {0};
    memset(g_http_client_response_buffer, 0, DYN_REG_RSP_BUFF_LEN);
    /**
     * NOTE: All the configuration parameters for http_client must be spefied either in URL or as host and path parameters.
     * If host and path parameters are not set, query parameter will be ignored. In such cases,
     * query parameter should be specified in URL.
     *
     * If URL as well as host and path parameters are specified, values of host and path will be considered.
     */
    ESP_LOGI(TAG, "_http_rest_with_url url = %s, post data = %s", url_string, request_json_str);
    esp_http_client_config_t config = {
        .url = url_string,
        // .host = host_string,
        // .path = path_string,
        // .query = query_string,
        .event_handler = _http_event_handler,
        // .user_data = local_response_buffer, // Pass address of local buffer to get response
        .disable_auto_redirect = false,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, request_json_str, strlen(request_json_str));
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "%s", g_http_client_response_buffer);
        esp_qcloud_dynreg_parse_response_payload(device_profile, g_http_client_response_buffer);
    }
    esp_http_client_cleanup(client);

    return err;
}

esp_err_t esp_qcloud_dynreg_device(void)
{
    const char *para_format =
        "{\"deviceName\":\"%s\",\"nonce\":%d,\"productId\":\"%s\",\"timestamp\":%d,\"signature\":\"%s\"}";
    uint32_t nonce;
    time_t timestamp;
    uint32_t length;
    char signature[DYN_REG_SIGN_LEN] = {0};
    char *request_json_str = NULL;
    esp_err_t ret = ESP_FAIL;
    esp_qcloud_profile_t* device_profile = esp_qcloud_get_device_profile();

    if (device_profile == NULL || strlen(device_profile->product_secret) < UTILS_AES_BLOCK_LEN)
    {
        ESP_LOGE(TAG, "Product key illegal!");
        goto EXIT;
    }

    srand_d(get_time_ms());
    nonce = rand_d();
    timestamp = time(0);

    if (ESP_OK != esp_qcloud_dynreg_generate_signature(device_profile, signature, DYN_REG_SIGN_LEN, nonce, timestamp))
    {
        ESP_LOGE(TAG, "generate_dynamic_register_sign fail");
        goto EXIT;
    }

    length = strlen(para_format) + strlen(device_profile->product_id) + strlen(device_profile->device_name) + sizeof(uint32_t) +
             sizeof(time_t) + strlen(signature) + DYN_BUFF_DATA_MORE;
    request_json_str = ESP_QCLOUD_MALLOC(length);
    if (!request_json_str)
    {
        ESP_LOGE(TAG, "malloc request memory fail");
        goto EXIT;
    }
    memset(request_json_str, 0, length);
    snprintf(request_json_str, length, para_format, device_profile->device_name, nonce, device_profile->product_id, timestamp, signature);
    ret = _http_rest_with_url(device_profile, DYN_REG_SERVER_URL, request_json_str);

EXIT:
    ESP_QCLOUD_FREE(request_json_str);

    return ret;
}
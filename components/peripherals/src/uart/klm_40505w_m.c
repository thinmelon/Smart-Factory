#include "esp_log.h"
#include "esp_event.h"
#include "klm_40505w_m.h"

#define FRAME_BEGIN_HEX "BB"
#define FRAME_END_HEX "7E"

static char g_frame[CMD_BUF_SIZE * 2] = {0};
static uint8_t g_frame_hex[CMD_BUF_SIZE] = {0};
static uint8_t g_frame_hex_length = 0;

/**
 * Convert command frame to HEX array
 *
 * @return ESP_OK or ESP_FAIL
 */
static esp_err_t convert_frame_to_hex_array(void)
{
    char tmp[3];
    g_frame_hex_length = strlen(g_frame);
    if (g_frame_hex_length % 2 != 0)
    {
        return ESP_FAIL;
    }
    printf("frame: %s legnth: %d\n", g_frame, g_frame_hex_length);

    memset(g_frame_hex, 0, CMD_BUF_SIZE);
    // printf("frame hex: ");
    for (uint8_t i = 0, j = 0; i < g_frame_hex_length; i += 2, j++)
    {
        memset(tmp, 0, 3);
        substr(tmp, g_frame, i, 2);
        g_frame_hex[j] = strtol(tmp, NULL, 16);
        // printf("%d \t", g_frame_hex[j]);
    }
    // printf("\n");

    g_frame_hex_length /= 2;
    return ESP_OK;
}

/**
 * Build frame according to datasheet
 *
 * @param frame_type
 * @param cmd_code
 * @param data
 *
 */
static esp_err_t build_frame(uint8_t frame_type, uint8_t cmd_code, const char *parameter)
{
    char check_sum[3] = {0};
    char frame_type_str[3] = {0};
    char cmd_code_str[3] = {0};
    char parameter_length_str[5] = {0};
    char frame_parameter[CMD_BUF_SIZE] = {0};

    hex2str(frame_type, frame_type_str, 2);
    hex2str(cmd_code, cmd_code_str, 2);

    strcpy(frame_parameter, parameter);
    replace(frame_parameter, " ", "");

    size_t parameter_length = strlen(frame_parameter);

    if (parameter_length == 1)
    {
        frame_parameter[1] = frame_parameter[0];
        frame_parameter[0] = '0';
        frame_parameter[2] = '\0';
    }
    else
    {
        if (parameter_length >= CMD_BUF_SIZE || (parameter_length % 2 != 0))
        {
            return ESP_FAIL;
        }
        frame_parameter[parameter_length] = '\0';
    }

    hex2str(strlen(frame_parameter) / 2, parameter_length_str, 4);

    memset(g_frame, 0, CMD_BUF_SIZE * 2);
    strcat(g_frame, FRAME_BEGIN_HEX);
    strcat(g_frame, frame_type_str);
    strcat(g_frame, cmd_code_str);
    strcat(g_frame, parameter_length_str);
    strcat(g_frame, frame_parameter);

    calculate_check_sum(g_frame + 2, check_sum); //  FRAME_BEGIN_HEX do not count in the calculation
    strcat(g_frame, check_sum);
    // printf("check sum: %s\n", check_sum);
    strcat(g_frame, FRAME_END_HEX);

    return convert_frame_to_hex_array();
}

int klm_write_bytes(void)
{
    int txBytes = uart_write_bytes(EX_UART_NUM, g_frame_hex, g_frame_hex_length);
    ESP_ERROR_CHECK(uart_wait_tx_done(EX_UART_NUM, 100)); // wait timeout is 100 RTOS ticks (TickType_t)
    return txBytes;
}

int klm_read_bytes(uint8_t *data, char *epc_str)
{
    char tmp[3] = {0};
    uint32_t check_sum = 0;
    bzero(epc_str, BUF_SIZE);

    if (data[1] == FRAME_TYPE_INFO && data[2] == CMD_INVENTORY) // Succeed to Read EPC
    {
        int rssidBm = data[5]; // rssidBm is negative && in bytes
        if (rssidBm > 127)
        {
            rssidBm = -((-rssidBm) & 0xFF);
        }
        /** RSSI 反映的是芯片输入端信号大小，不包含天线增益和定向耦合器衰减等。
         *  RSSI 为十六进制有符号数，单位为 dBm */
        printf("rssi[dBm]: %d\n", rssidBm);

        int pc_epc_length = (data[6] / 8 + 1) * 2 - 2;
        printf("EPC length: %d\n", pc_epc_length);
        /** 读取到异常值 EPC编码通常是96bit或64bit，这个长度可以在编码过程中自定义，但要考虑标签存储容量 */
        if (pc_epc_length <= 0 || pc_epc_length > 12)
        {
            return -1;  //  EPC码长度小于等于0，或者大于12字节
        }
        
        for (uint8_t i = 1; i <= 10 + pc_epc_length; i++)
        {
            check_sum += data[i];
        }
        check_sum = check_sum % 256;
        printf("Checksum: %d\n", check_sum);
        if(check_sum != data[11 + pc_epc_length])
        {
            return -1;  //  校验和不正确
        }
        
        for (uint8_t i = 0; i < pc_epc_length; i++)
        {
            memset(tmp, 0, 3);
            hex2str(data[8 + i], tmp, 2);
            // printf("%d|%s\t", data[8 + i], tmp);
            strcat(epc_str, tmp);
        }
        // printf("\n");
        printf("EPC: %s\n", epc_str);
        printf("CRC: %d  %d\n", data[8 + pc_epc_length], data[9 + pc_epc_length]);
        printf("ANT: %d\n", data[10 + pc_epc_length]);        
        return 0;
    }

    return -1;
}

esp_err_t klm_get_module_info(uint8_t info_type)
{
    char info_type_str[3] = {0};
    hex2str(info_type, info_type_str, 2);
    return build_frame(FRAME_TYPE_CMD, CMD_GET_MODULE_INFO, info_type_str);
}

/**
 * @brief 设置工作地区
 *
 * @param region
 * @return esp_err_t
 */
esp_err_t klm_set_region(uint8_t region)
{
    char region_str[3] = {0};
    hex2str(region, region_str, 2);
    return build_frame(FRAME_TYPE_CMD, CMD_SET_REGION, region_str);
}

/**
 * @brief 设置工作信道
 *
 * @param channel
 * @return esp_err_t
 */
esp_err_t klm_set_rf_channel(uint8_t channel)
{
    char channel_str[3] = {0};
    hex2str(channel, channel_str, 2);
    return build_frame(FRAME_TYPE_CMD, CMD_SET_RF_CHANNEL, channel_str);
}

/**
 * @brief 获取工作信道
 *
 * @return esp_err_t
 */
esp_err_t klm_get_rf_channel()
{
    return build_frame(FRAME_TYPE_CMD, CMD_GET_RF_CHANNEL, "");
}

/**
 * @brief 设置自动跳频
 *
 * @param fhss 0xFF为设置自动跳频，0x00为取消自动跳频
 * @return esp_err_t
 */
esp_err_t klm_set_fhss(uint8_t fhss)
{
    char fhss_str[3] = {0};
    hex2str(fhss, fhss_str, 2);
    return build_frame(FRAME_TYPE_CMD, CMD_SET_FHSS, fhss_str);
}

/**
 * @brief 设置发射功率
 *
 * @param power 如0x07D0，设置功率为十进制2000，即20dBm
 * @return esp_err_t
 */
esp_err_t klm_set_pa_power(uint16_t power)
{
    char power_dbm_str[5] = {0};
    hex2str(power, power_dbm_str, 4);
    return build_frame(FRAME_TYPE_CMD, CMD_SET_POWER, power_dbm_str);
}

/**
 * @brief 获取发射功率
 *
 * @return esp_err_t
 */
esp_err_t klm_get_pa_power(void)
{
    return build_frame(FRAME_TYPE_CMD, CMD_GET_POWER, "");
}

/**
 * @brief 设置自动跳频
 *
 * @param cw
 * @return esp_err_t
 */
esp_err_t klm_set_carrier_wave(uint8_t cw)
{
    char carrier_wave_str[3] = {0};
    hex2str(cw, carrier_wave_str, 2);
    return build_frame(FRAME_TYPE_CMD, CMD_SET_CW, carrier_wave_str);
}

/**
 * @brief 单次轮询指令
 *
 * @return esp_err_t
 */
esp_err_t klm_read_single_tag_id(void)
{
    return build_frame(FRAME_TYPE_CMD, CMD_INVENTORY, "");
}

/**
 * @brief 多次轮询指令
 *
 * @param loop_number 轮询次数
 * @return esp_err_t
 */
esp_err_t klm_read_multi_tag_id(uint16_t loop_number)
{
    char loop_number_str[5] = {0};
    char frame_parameter[CMD_BUF_SIZE] = {0};

    if (loop_number == 0)
    {
        return ESP_FAIL;
    }
    hex2str(loop_number, loop_number_str, 4);
    strcat(frame_parameter, "22");            //  Reserved words
    strcat(frame_parameter, loop_number_str); //  Loop number in HEX

    return build_frame(FRAME_TYPE_CMD, CMD_READ_MULTI, frame_parameter);
}

/**
 * @brief 停止多次轮询指令
 *
 * @return esp_err_t
 */
esp_err_t klm_stop_read_multi_tag_id(void)
{
    return build_frame(FRAME_TYPE_CMD, CMD_STOP_MULTI, "");
}

/**
 * @brief 设置 Select 参数指令
 *
 * @note  在多标签的情况下，可以根据 Select 参数只对特定标签进行轮询和读写等操作。
 *
 * @param target Target(3 bit)     S0(000)、S1(001)、S2(010)、S3(011)、SL(100)
 * @param action Action(3 bit)    Reference to ISO18000-6C
 * @param memory_bank Memory bank(2 bit)    RFU(00)、EPC(01)、TID(10)、USR(11)
 * @param pointer Pointer(32 bit)     Start Address
 * @param length Length(8 bit)
 * @param mask Mask(0-255bit)   Mask Data according to Length
 * @param truncated Truncate(1 bit)   Disable(0)、Enable(1)
 * @return esp_err_t
 */
esp_err_t klm_set_select(uint8_t target, uint8_t action, uint8_t memory_bank, uint32_t pointer, uint8_t length, const char *mask, uint8_t truncated)
{
    char combine_byte_str[3] = {0};
    char pointer_str[9] = {0};
    char length_str[3] = {0};
    char truncated_str[3] = {0};
    char data_field[40] = {0};
    uint8_t combine_byte = (target << 5) | (action << 2) | memory_bank;
    hex2str(combine_byte, combine_byte_str, 2);
    hex2str(pointer, pointer_str, 8);
    hex2str(length, length_str, 2);
    hex2str(truncated, truncated_str, 2);

    strcat(data_field, combine_byte_str);
    strcat(data_field, pointer_str);
    strcat(data_field, length_str);
    strcat(data_field, truncated_str);
    strcat(data_field, mask);

    replace(data_field, " ", "");

    return build_frame(FRAME_TYPE_CMD, CMD_SET_SELECT_PARA, data_field);
}

/**
 * @brief 设置 Select 模式
 *
 * @param mode Select 模式 Mode 含义：
 * 0x00: 在对标签的所有操作之前都预先发送 Select 指令选取特定的标签。
 * 0x01: 在对标签操作之前不发送 Select 指令。
 * 0x02: 仅对除轮询 Inventory 之外的标签操作之前发送 Select 指令，如在Read，Write，Lock，Kill 之前先通过 Select 选取特定的标签。
 *
 * @return esp_err_t
 */
esp_err_t klm_set_select_mode(uint8_t mode)
{
    char mode_str[3] = {0};
    hex2str(mode, mode_str, 2);
    return build_frame(FRAME_TYPE_CMD, CMD_SET_INVENTORY_MODE, mode_str);
}

/**
 * @brief 读取标签数据
 * 
 * @param access_password 如果 Access Password全为零 "00 00 00 00"，则不发送Access指令。
 * @param memory_bank 0x00-RFU 0x01-EPC 0x02-TID 0x03-User
 * @param start_address 标签数据区地址偏移，单位：Word，即2个字节
 * @param data_length 要读取标签数据存储区的长度，单位：Word，即2个字节
 * @return esp_err_t 
 */
esp_err_t klm_read_tag_data(const char *access_password, uint8_t memory_bank, uint16_t start_address, uint16_t data_length)
{
    char memory_bank_str[3] = {0};
    char start_address_str[5] = {0};
    char data_length_str[5] = {0};
    char data_field[20] = {0};
    if (strlen(access_password) != 8)
    {
        return ESP_FAIL;
    }

    hex2str(memory_bank, memory_bank_str, 2);
    hex2str(start_address, start_address_str, 4);
    hex2str(data_length, data_length_str, 4);

    strcat(data_field, access_password);
    strcat(data_field, memory_bank_str);
    strcat(data_field, start_address_str);
    strcat(data_field, data_length_str);

    return build_frame(FRAME_TYPE_CMD, CMD_READ_DATA, data_field);
}

esp_err_t klm_write_tag_data(const char *access_password, uint8_t memory_bank, uint16_t start_address, uint16_t data_length, const char *data)
{
    char memory_bank_str[3] = {0};
    char start_address_str[5] = {0};
    char data_length_str[5] = {0};
    char data_field[28] = {0};

    if (strlen(access_password) != 8)
    {
        return ESP_FAIL;
    }

    strcat(data_field, access_password);
    strcat(data_field, memory_bank_str);
    strcat(data_field, start_address_str);
    strcat(data_field, data_length_str);
    strcat(data_field, data);

    return build_frame(FRAME_TYPE_CMD, CMD_WRITE_DATA, data_field);
}

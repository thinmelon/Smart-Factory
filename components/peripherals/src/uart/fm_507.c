#include "esp_log.h"
#include "esp_event.h"
#include "fm_507.h"

// static char g_frame[CMD_BUF_SIZE * 2] = {0};
static uint8_t g_frame_hex[CMD_BUF_SIZE] = {0};
static uint8_t g_frame_hex_length = 0;
static const char *TAG = "fm_507";

static esp_err_t fm507_build_frame(uint8_t cmd_code, const char *parameter)
{
    char tmp[3];
    char frame_parameter[CMD_BUF_SIZE] = {0};
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

    memset(g_frame_hex, 0, CMD_BUF_SIZE);
    g_frame_hex[0] = FM_FRAME_BEGIN_BYTE;
    g_frame_hex[1] = cmd_code;
    g_frame_hex_length = 1 + 1;
    parameter_length = strlen(frame_parameter);
    for (uint8_t i = 0; i < parameter_length; i += 2, g_frame_hex_length++)
    {
        memset(tmp, 0, 3);
        substr(tmp, frame_parameter, i, 2);
        g_frame_hex[g_frame_hex_length] = strtol(tmp, NULL, 16);
    }
    g_frame_hex[g_frame_hex_length] = FM_FRAME_END_BYTE;
    g_frame_hex_length++;

    // for (uint8_t j = 0; j < g_frame_hex_length; j++)
    //     printf("%0x \t", g_frame_hex[j]);
    // printf("\r\n");

    return ESP_OK;
}

int fm507_write_bytes(void)
{
    int txBytes = uart_write_bytes(EX_UART_NUM, g_frame_hex, g_frame_hex_length);
    ESP_ERROR_CHECK(uart_wait_tx_done(EX_UART_NUM, 100)); // wait timeout is 100 RTOS ticks (TickType_t)
    return txBytes;
}

void fm507_get_firmware_version(void)
{
    fm507_build_frame(FM_CMD_GET_FW_VERSION, "");
}

void fm507_get_reader_id(void)
{
    fm507_build_frame(FM_CMD_GET_READ_ID, "");
}

void fm507_query_epc(void)
{
    fm507_build_frame(FM_CMD_QUERY_EPC, "");
}

void fm507_multi_epc(void)
{
    fm507_build_frame(FM_CMD_QUERY_MULTI_EPC, "");
}

void fm507_read_power(void)
{
    fm507_build_frame(FM_CMD_POWER, "30 2C 30 30");
}

/**
 * @brief Read TID bank address=0 word=6
 *       <LF>R2,0,6<CR>
 *
 * @param address   HEX format
 * @param word      HEX format
 */
void fm507_read_TID_bank(uint32_t address, uint32_t word)
{
    // char address_str[3] = {0};
    // char word_str[5] = {0};
    // uint32_t tmp_value = word;
    // uint8_t count = 0;
    // while (tmp_value > 0)
    // {
    //     tmp_value >>= 4;
    //     count++;
    // }
    // hex2str(address, address_str, 2);
    // hex2str(word, word_str, count * 2);

    char params[CMD_BUF_SIZE] = {0};
    strcat(params, "32"); //  2
    strcat(params, "2C"); //  ,
    strcat(params, "30"); //  address
    strcat(params, "2C"); //  ,
    strcat(params, "36"); //  word

    fm507_build_frame(FM_CMD_READ_BANK, params);
}
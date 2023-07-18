#ifndef __KLM_40505W_M_H__
#define __KLM_40505W_M_H__

#include "uart_utils.h"

// KLM Series firmware commands. Described in chapter 4 of the datasheet.
typedef enum KLM_FRAME
{
    FRAME_BEGIN_BYTE = 0xBB, //  Packet Begin Byte
    FRAME_END_BYTE = 0x7E,   //  Packet End Byte
    FRAME_TYPE_CMD = 0x00,   //  Packet Type : Command
    FRAME_TYPE_ANS = 0x01,   //  Packet Type : Answer
    FRAME_TYPE_INFO = 0x02   //  Packet Type ：Message
} KLM_FRAME;

typedef enum KLM_COMMAND_TYPE
{
    CMD_GET_MODULE_INFO = 0x03,       //  Get Module Information
    CMD_SET_QUERY = 0x0E,             //  Set Query Parameters
    CMD_GET_QUERY = 0x0D,             //  Get Query Parameters
    CMD_INVENTORY = 0x22,             //  Read Single Tag ID(PC + EPC)
    CMD_READ_MULTI = 0x27,            //  Read Multiply Tag IDs(PC + EPC)
    CMD_STOP_MULTI = 0x28,            //  Stop Read Multiply Tag IDs(PC + EPC)
    CMD_READ_DATA = 0x39,             //  Read Tag Data
    CMD_WRITE_DATA = 0x49,            //  Write Tag Data
    CMD_LOCK_UNLOCK = 0x82,           //  Lock/Unlock Tag Memory
    CMD_KILL = 0x65,                  //  Kill Tag
    CMD_SET_REGION = 0x07,            //  Set Reader RF Region
    CMD_SET_RF_CHANNEL = 0xAB,        //  Set Reader RF Channel
    CMD_GET_RF_CHANNEL = 0xAA,        //  Get Reader RF Channel No.
    CMD_SET_POWER = 0xB6,             //  Set Reader Power Level
    CMD_GET_POWER = 0xB7,             //  Get Reader Power Level
    CMD_SET_FHSS = 0xAD,              //  Set Reader FHSS On/Off
    CMD_SET_CW = 0xB0,                //  Set Reader CW On/Off
    CMD_SET_MODEM_PARA = 0xF0,        //  Set Modem Parameter
    CMD_READ_MODEM_PARA = 0xF1,       //  Read Modem Parameter
    CMD_SET_SELECT_PARA = 0x0C,       //  Set ISO18000-6C Select Command Parameters
    CMD_GET_SELECT_PARA = 0x0B,       //  Get Select Command Parameters
    CMD_SET_INVENTORY_MODE = 0x12,    //  Set Inventory Mode
                                      //  (MODE0, Send Select Command Before Each Tag Command)
                                      //  (MODE1, DoNot Send Select Command Before Each Tag Command)
                                      //  (MODE2, Send Select Command Before Tag Commands(Read, Write, Lock, Kill) Except Inventory
    CMD_SCAN_JAMMER = 0xF2,           //  Scan Jammer
    CMD_SCAN_RSSI = 0xF3,             //  Scan RSSI
    CMD_IO_CONTROL = 0x1A,            //  Control IO
    CMD_RESTART = 0x19,               //  Restart Reader
    CMD_SET_READER_ENV_MODE = 0xF5,   //  Set Reader Mode(Dense Reader Mode or High-sensitivity Mode)
    CMD_INSERT_FHSS_CHANNEL = 0xA9,   //  Insert RF Channel to the FHSS Frequency Look-up Table
    CMD_SLEEP_MODE = 0x17,            //  Set Reader to Sleep Mode
    CMD_SET_SLEEP_TIME = 0x1D,        //  Set Reader Idle Time, after These Minutes, the Reader Will Go to Sleep Mode
    CMD_LOAD_NV_CONFIG = 0x0A,        //  Load Configuration From Non-volatile Memory
    CMD_SAVE_NV_CONFIG = 0x09,        //  Save Configuration to Non-volatile Memory
    CMD_NXP_CHANGE_CONFIG = 0xE0,     //  Change Config Command for NXP G2X Tags
    CMD_NXP_READPROTECT = 0xE1,       //  ReadProtect Command for NXP G2X Tags
    CMD_NXP_RESET_READPROTECT = 0xE2, //  Reset ReadProtect Command for NXP G2X Tags
    CMD_NXP_CHANGE_EAS = 0xE3,        //  ChangeEAS Command for NXP G2X Tags
    CMD_NXP_EAS_ALARM = 0xE4          //  EAS_Alarm Command for NXP G2X Tags
} KLM_COMMAND_TYPE;

typedef enum KLM_FAIL_TYPE
{
    CMD_EXE_FAILED = 0xFF,             /// Command Execute Fail Type
    FAIL_INVALID_PARA = 0x0E,          /// Fail Type : Command Parameter Invalid
    FAIL_INVENTORY_TAG_TIMEOUT = 0x15, /// Fail Type : Read Tag ID Time out
    FAIL_INVALID_CMD = 0x17,           /// Fail Type : Invalid Command
    FAIL_FHSS_FAIL = 0x20,             /// Fail Type : FHSS Failed
    FAIL_ACCESS_PWD_ERROR = 0x16,      /// Fail Type : Access Password Error
    FAIL_READ_MEMORY_NO_TAG = 0x09,    /// Fail Type : Read Tag Memory No Tag Response
    FAIL_READ_ERROR_CODE_BASE = 0xA0,  /// Fail Type : Error Code(defined in C1Gen2 Protocol) Caused By Read Operation.
                                       /// The Error Code Will Be Added to this Code.
    FAIL_WRITE_MEMORY_NO_TAG = 0x10,   /// Fail Type : Write Tag Memory No Tag Response
    FAIL_WRITE_ERROR_CODE_BASE = 0xB0, /// Fail Type : Error Code(defined in C1Gen2 Protocol) Caused By Write Operation.
                                       /// The Error Code Will Be Added to this Code.
    FAIL_LOCK_NO_TAG = 0x13,           /// Fail Type : Lock Tag No Tag Response
    FAIL_LOCK_ERROR_CODE_BASE = 0xC0,  /// Fail Type : Error Code(defined in C1Gen2 Protocol) Caused By Lock Operation.
                                       /// The Error Code Will Be Added to this Code.
    FAIL_KILL_NO_TAG = 0x12,           /// Fail Type : Kill Tag No Tag Response
    FAIL_KILL_ERROR_CODE_BASE = 0xD0   /// Fail Type : Error Code(defined in C1Gen2 Protocol) Caused By Kill Operation.
                                       /// The Error Code Will Be Added to this Code.
} KLM_FAIL_TYPE;

typedef enum KLM_ERROR_CODE
{
    ERROR_CODE_OTHER_ERROR = 0x00,        /// Error Code(according to C1Gen2 Protocol) : Other Error
    ERROR_CODE_MEM_OVERRUN = 0x03,        /// Error Code(according to C1Gen2 Protocol) : Memory Overrun
    ERROR_CODE_MEM_LOCKED = 0x04,         /// Error Code(according to C1Gen2 Protocol) : Memory Locked
    ERROR_CODE_INSUFFICIENT_POWER = 0x0B, /// Error Code(according to C1Gen2 Protocol) : Insufficient Power
    ERROR_CODE_NON_SPEC_ERROR = 0x0F      /// Error Code(according to C1Gen2 Protocol) : Non-specific Error
} KLM_ERROR_CODE;

typedef enum KLM_REGION_CODE
{
    REGION_CODE_CHN2 = 0x01,  /// Region Code : China 2 (920MHz - 925MHz)
    REGION_CODE_US = 0x02,    /// Region Code : US
    REGION_CODE_EUR = 0x03,   /// Region Code : Europe
    REGION_CODE_CHN1 = 0x04,  /// Region Code : China 1(840MHz - 845MHz)
    REGION_CODE_JAPAN = 0x05, /// Region Code : Japan (Not Support Yet)
    REGION_CODE_KOREA = 0x06  /// Region Code : Korea
} KLM_REGION_CODE;

typedef enum KLM_INVENTORY_MODE
{
    INVENTORY_MODE0 = 0x00, /// Inventory MODE0, Send Select Command Before Each Tag Command
    INVENTORY_MODE1 = 0x01, /// Inventory MODE1, DoNot Send Select Command Before Each Tag Command
    INVENTORY_MODE2 = 0x02  /// Inventory MODE1, Send Select Command Before Tag Commands(Read, Write, Lock, Kill etc.) Except Inventory
} KLM_INVENTORY_MODE;

typedef enum KLM_MODULE_FIELD
{
    MODULE_HARDWARE_VERSION_FIELD = 0x00, /// Get Module Information Type : Hardware Version
    MODULE_SOFTWARE_VERSION_FIELD = 0x01, /// Get Module Information Type : Software Version
    MODULE_MANUFACTURE_INFO_FIELD = 0x02  /// Get Module Information Type : Manufacture Information
} KLM_MODULE_FIELD;

/**
 * @brief UART ISR handler.
 *
 */
// typedef void (*klm_receive_message)(void *);

void klm_uart_init(void);

/**
 * @brief 获取模组信息
 *
 * @return esp_err_t
 */
esp_err_t klm_get_module_info(uint8_t info_type);

/**
 * @brief 单次轮询指令
 *
 * @return esp_err_t
 */
esp_err_t klm_read_single_tag_id(void);

/**
 * @brief 多次轮询指令
 *
 * @param loop_number 轮询次数
 * @return esp_err_t
 */
esp_err_t klm_read_multi_tag_id(uint16_t loop_number);

/**
 * @brief 停止多次轮询指令
 *
 * @return esp_err_t
 */
esp_err_t klm_stop_read_multi_tag_id(void);

/**
 * @brief 获取发射功率
 *
 * @return esp_err_t
 */
esp_err_t klm_get_pa_power(void);

/**
 * @brief 设置发射功率
 *
 * @param power 如0x07D0，设置功率为十进制2000，即20dBm
 * @return esp_err_t
 */
esp_err_t klm_set_pa_power(uint16_t power);

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
esp_err_t klm_set_select(uint8_t target, uint8_t action, uint8_t memory_bank, uint32_t pointer, uint8_t length, const char *mask, uint8_t truncated);

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
esp_err_t klm_set_select_mode(uint8_t mode);

/**
 * @brief 读取标签数据
 * 
 * @param access_password 如果 Access Password全为零 "00 00 00 00"，则不发送Access指令。
 * @param memory_bank 0x00-RFU 0x01-EPC 0x02-TID 0x03-User
 * @param start_address 标签数据区地址偏移，单位：Word，即2个字节
 * @param data_length 要读取标签数据存储区的长度，单位：Word，即2个字节
 * @return esp_err_t 
 */
esp_err_t klm_read_tag_data(const char *access_password, uint8_t memory_bank, uint16_t start_address, uint16_t data_length);


/**
 * @brief 
 * 
 * @return int 
 */
int klm_write_bytes(void);

/**
 * @brief 
 * 
 * @param data 
 * @param epc_str 
 * @return int 
 */
int klm_read_bytes(uint8_t *data, char* epc_str);

#endif /** __KLM_40505W_M_H__ */
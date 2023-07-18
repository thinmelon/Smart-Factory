#ifndef __FM_507_H__
#define __FM_507_H__

#include "uart_utils.h"

// FM Series firmware commands.
typedef enum FM_FRAME
{
    FM_FRAME_BEGIN_BYTE = 0x0A, //  Packet Begin Byte
    FM_FRAME_END_BYTE = 0x0D    //  Packet End Byte
} FM_FRAME;

typedef enum FM_COMMAND_TYPE
{
    FM_CMD_GET_FW_VERSION = 0x56,  //  FW Version
    FM_CMD_GET_READ_ID = 0x53,     //  Reader ID
    FM_CMD_QUERY_EPC = 0x51,       //  Query EPC
    FM_CMD_QUERY_MULTI_EPC = 0x55, //  Multi  EPC
    FM_CMD_POWER = 0x4E,           //  Read or write power
    FM_CMD_READ_BANK = 0x52,       //  Read TID/EPC/USER/Reserved bank
    FM_CMD_WRITE_BANK = 0x57,      //  Write EPC/USER/Reserved bank or pwd
    FM_CMD_ACCESS_PWD = 0x50,      //  Access password
    FM_CMD_KILL = 0x4B,            //  Kill
    FM_CMD_LOCK_MASK = 0x4C,       //  Lock mask
    FM_CMD_SET_MODE = 0x4E         //  US/TW/CN1/CN2/CE/JP/KR/VIN/EN2/IN mode
} FM_COMMAND_TYPE;

/**
 * @brief send command/data to rfid reader
 *
 * @return int
 */
int fm507_write_bytes(void);

/**
 * @brief get module's info
 *
 */
void fm507_get_firmware_version(void);

/**
 * @brief
 *
 */
void fm507_get_reader_id(void);

/**
 * @brief
 *
 */
void fm507_query_epc(void);

/**
 * @brief
 *
 */
void fm507_multi_epc(void);

/**
 * @brief 
 * 
 */
void fm507_read_power(void);

/**
 * @brief Read TID bank address=0 word=6
 *       <LF>R2,0,6<CR>
 * 
 * @param address 
 * @param word 
 */
void fm507_read_TID_bank(uint32_t address, uint32_t word);

#endif /** __FM_507_H__ */
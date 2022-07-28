#ifndef __LORA_H__
#define __LORA_H__

/*
 * Register definitions
 */
#define REG_FIFO 0x00
#define REG_OP_MODE 0x01
#define REG_FRF_MSB 0x06
#define REG_FRF_MID 0x07
#define REG_FRF_LSB 0x08
// Tx settings
#define REG_PA_CONFIG 0x09
#define REG_PARAMP 0x0A
#define REG_OCP 0x0B
// Rx settings
#define REG_LR_LNA 0x0C
// LoRa registers
#define REG_LR_FIFO_ADDRESS_PTR 0x0D
#define REG_LR_FIFO_TX_BASE_ADDRESS 0x0E
#define REG_LR_FIFO_RX_BASE_ADDRESS 0x0F
#define REG_LR_FIFO_RX_CURRENT_ADDRESS 0x10
#define REG_LR_IRQ_FLAGS_MASK 0x11
#define REG_LR_IRQFLAGS 0x12
#define REG_LR_RXNBBYTES 0x13
#define REG_LR_RXHEADERCNTVALUEMSB 0x14
#define REG_LR_RXHEADERCNTVALUELSB 0x15
#define REG_LR_RXPACKETCNTVALUEMSB 0x16
#define REG_LR_RXPACKETCNTVALUELSB 0x17
#define REG_LR_MODEMSTAT 0x18
#define REG_LR_PKTSNRVALUE 0x19
#define REG_LR_PKTRSSIVALUE 0x1A
#define REG_LR_RSSIVALUE 0x1B
#define REG_LR_HOPCHANNEL 0x1C
#define REG_LR_MODEMCONFIG1 0x1D
#define REG_LR_MODEMCONFIG2 0x1E
#define REG_LR_SYMBTIMEOUTLSB 0x1F
#define REG_LR_PREAMBLEMSB 0x20
#define REG_LR_PREAMBLELSB 0x21
#define REG_LR_PAYLOADLENGTH 0x22
#define REG_LR_PAYLOADMAXLENGTH 0x23
#define REG_LR_HOPPERIOD 0x24
#define REG_LR_FIFORXBYTEADDR 0x25
#define REG_LR_MODEMCONFIG3 0x26
#define REG_LR_FEIMSB 0x28
#define REG_LR_FEIMID 0x29
#define REG_LR_FEILSB 0x2A
#define REG_LR_RSSIWIDEBAND 0x2C
#define REG_LR_TEST2F 0x2F
#define REG_LR_TEST30 0x30
#define REG_LR_DETECTOPTIMIZE 0x31
#define REG_LR_INVERTIQ 0x33
#define REG_LR_TEST36 0x36
#define REG_LR_DETECTIONTHRESHOLD 0x37
#define REG_LR_SYNCWORD 0x39
#define REG_LR_TEST3A 0x3A
#define REG_LR_INVERTIQ2 0x3B
// FSK Service settings
#define REG_IMAGECAL 0x3B
#define REG_TEMP 0x3C
#define REG_LOWBAT 0x3D
// I/O settings
#define REG_DIOMAPPING1 0x40
#define REG_DIOMAPPING2 0x41
// Version
#define REG_VERSION 0x42
// Additional settings
#define REG_LR_PLLHOP 0x44
#define REG_LR_TCXO 0x4B
#define REG_LR_PADAC 0x4D
#define REG_LR_FORMERTEMP 0x5B
#define REG_LR_BITRATEFRAC 0x5D
#define REG_LR_AGCREF 0x61
#define REG_LR_AGCTHRESH1 0x62
#define REG_LR_AGCTHRESH2 0x63
#define REG_LR_AGCTHRESH3 0x64
#define REG_LR_PLL 0x70

/*!
 * RegImageCal
 */
#define RF_IMAGECAL_AUTOIMAGECAL_MASK 0x7F
#define RF_IMAGECAL_AUTOIMAGECAL_ON 0x80
#define RF_IMAGECAL_AUTOIMAGECAL_OFF 0x00 // Default

#define RF_IMAGECAL_IMAGECAL_MASK 0xBF
#define RF_IMAGECAL_IMAGECAL_START 0x40

#define RF_IMAGECAL_IMAGECAL_RUNNING 0x20
#define RF_IMAGECAL_IMAGECAL_DONE 0x00 // Default

#define RF_IMAGECAL_TEMPCHANGE_HIGHER 0x08
#define RF_IMAGECAL_TEMPCHANGE_LOWER 0x00

#define RF_IMAGECAL_TEMPTHRESHOLD_MASK 0xF9
#define RF_IMAGECAL_TEMPTHRESHOLD_05 0x00
#define RF_IMAGECAL_TEMPTHRESHOLD_10 0x02 // Default
#define RF_IMAGECAL_TEMPTHRESHOLD_15 0x04
#define RF_IMAGECAL_TEMPTHRESHOLD_20 0x06

#define RF_IMAGECAL_TEMPMONITOR_MASK 0xFE
#define RF_IMAGECAL_TEMPMONITOR_ON 0x00 // Default
#define RF_IMAGECAL_TEMPMONITOR_OFF 0x01

/*
 * Transceiver modes
 */
#define RFLR_OPMODE_LONGRANGEMODE_MASK 0x7F
#define RFLR_OPMODE_LONGRANGEMODE_OFF 0x00 // Default
#define RFLR_OPMODE_LONGRANGEMODE_ON 0x80

#define RFLR_OPMODE_ACCESSSHAREDREG_MASK 0xBF
#define RFLR_OPMODE_ACCESSSHAREDREG_ENABLE 0x40
#define RFLR_OPMODE_ACCESSSHAREDREG_DISABLE 0x00 // Default

#define RFLR_OPMODE_FREQMODE_ACCESS_MASK 0xF7
#define RFLR_OPMODE_FREQMODE_ACCESS_LF 0x08 // Default
#define RFLR_OPMODE_FREQMODE_ACCESS_HF 0x00

#define RFLR_OPMODE_MASK 0xF8
#define RFLR_OPMODE_SLEEP 0x00
#define RFLR_OPMODE_STANDBY 0x01 // Default
#define RFLR_OPMODE_SYNTHESIZER_TX 0x02
#define RFLR_OPMODE_TRANSMITTER 0x03
#define RFLR_OPMODE_SYNTHESIZER_RX 0x04
#define RFLR_OPMODE_RECEIVER 0x05
// LoRa specific modes
#define RFLR_OPMODE_RECEIVER_SINGLE 0x06
#define RFLR_OPMODE_CAD 0x07

/*!
 * RegModemConfig1
 */
#define RFLR_MODEMCONFIG1_BW_MASK 0x0F
#define RFLR_MODEMCONFIG1_BW_7_81_KHZ 0x00
#define RFLR_MODEMCONFIG1_BW_10_41_KHZ 0x10
#define RFLR_MODEMCONFIG1_BW_15_62_KHZ 0x20
#define RFLR_MODEMCONFIG1_BW_20_83_KHZ 0x30
#define RFLR_MODEMCONFIG1_BW_31_25_KHZ 0x40
#define RFLR_MODEMCONFIG1_BW_41_66_KHZ 0x50
#define RFLR_MODEMCONFIG1_BW_62_50_KHZ 0x60
#define RFLR_MODEMCONFIG1_BW_125_KHZ 0x70 // Default
#define RFLR_MODEMCONFIG1_BW_250_KHZ 0x80
#define RFLR_MODEMCONFIG1_BW_500_KHZ 0x90

#define RFLR_MODEMCONFIG1_CODINGRATE_MASK 0xF1
#define RFLR_MODEMCONFIG1_CODINGRATE_4_5 0x02
#define RFLR_MODEMCONFIG1_CODINGRATE_4_6 0x04 // Default
#define RFLR_MODEMCONFIG1_CODINGRATE_4_7 0x06
#define RFLR_MODEMCONFIG1_CODINGRATE_4_8 0x08

#define RFLR_MODEMCONFIG1_IMPLICITHEADER_MASK 0xFE
#define RFLR_MODEMCONFIG1_IMPLICITHEADER_ON 0x01
#define RFLR_MODEMCONFIG1_IMPLICITHEADER_OFF 0x00 // Default

/*!
 * RegModemConfig2
 */
#define RFLR_MODEMCONFIG2_SF_MASK 0x0F
#define RFLR_MODEMCONFIG2_SF_6 0x60
#define RFLR_MODEMCONFIG2_SF_7 0x70 // Default
#define RFLR_MODEMCONFIG2_SF_8 0x80
#define RFLR_MODEMCONFIG2_SF_9 0x90
#define RFLR_MODEMCONFIG2_SF_10 0xA0
#define RFLR_MODEMCONFIG2_SF_11 0xB0
#define RFLR_MODEMCONFIG2_SF_12 0xC0

#define RFLR_MODEMCONFIG2_TXCONTINUOUSMODE_MASK 0xF7
#define RFLR_MODEMCONFIG2_TXCONTINUOUSMODE_ON 0x08
#define RFLR_MODEMCONFIG2_TXCONTINUOUSMODE_OFF 0x00

#define RFLR_MODEMCONFIG2_RXPAYLOADCRC_MASK 0xFB
#define RFLR_MODEMCONFIG2_RXPAYLOADCRC_ON 0x04
#define RFLR_MODEMCONFIG2_RXPAYLOADCRC_OFF 0x00 // Default

#define RFLR_MODEMCONFIG2_SYMBTIMEOUTMSB_MASK 0xFC
#define RFLR_MODEMCONFIG2_SYMBTIMEOUTMSB 0x00 // Default

/*!
 * RegModemConfig3
 */
#define RFLR_MODEMCONFIG3_LOWDATARATEOPTIMIZE_MASK 0xF7
#define RFLR_MODEMCONFIG3_LOWDATARATEOPTIMIZE_ON 0x08
#define RFLR_MODEMCONFIG3_LOWDATARATEOPTIMIZE_OFF 0x00 // Default

#define RFLR_MODEMCONFIG3_AGCAUTO_MASK 0xFB
#define RFLR_MODEMCONFIG3_AGCAUTO_ON 0x04 // Default
#define RFLR_MODEMCONFIG3_AGCAUTO_OFF 0x00

/*!
 * RegDetectOptimize
 */
#define RFLR_DETECTIONOPTIMIZE_MASK 0xF8
#define RFLR_DETECTIONOPTIMIZE_SF7_TO_SF12 0x03 // Default
#define RFLR_DETECTIONOPTIMIZE_SF6 0x05

/*!
 * RegDetectionThreshold
 */
#define RFLR_DETECTIONTHRESH_SF7_TO_SF12 0x0A // Default
#define RFLR_DETECTIONTHRESH_SF6 0x0C

/*!
 * RegInvertIQ
 */
#define RFLR_INVERTIQ_RX_MASK 0xBF
#define RFLR_INVERTIQ_RX_OFF 0x00
#define RFLR_INVERTIQ_RX_ON 0x40
#define RFLR_INVERTIQ_TX_MASK 0xFE
#define RFLR_INVERTIQ_TX_OFF 0x01
#define RFLR_INVERTIQ_TX_ON 0x00

/*!
 * RegInvertIQ2
 */
#define RFLR_INVERTIQ2_ON 0x19
#define RFLR_INVERTIQ2_OFF 0x1D

/*!
 * RegDioMapping1
 */
#define RFLR_DIOMAPPING1_DIO0_MASK 0x3F
#define RFLR_DIOMAPPING1_DIO0_00 0x00 // Default
#define RFLR_DIOMAPPING1_DIO0_01 0x40
#define RFLR_DIOMAPPING1_DIO0_10 0x80
#define RFLR_DIOMAPPING1_DIO0_11 0xC0

#define RFLR_DIOMAPPING1_DIO1_MASK 0xCF
#define RFLR_DIOMAPPING1_DIO1_00 0x00 // Default
#define RFLR_DIOMAPPING1_DIO1_01 0x10
#define RFLR_DIOMAPPING1_DIO1_10 0x20
#define RFLR_DIOMAPPING1_DIO1_11 0x30

#define RFLR_DIOMAPPING1_DIO2_MASK 0xF3
#define RFLR_DIOMAPPING1_DIO2_00 0x00 // Default
#define RFLR_DIOMAPPING1_DIO2_01 0x04
#define RFLR_DIOMAPPING1_DIO2_10 0x08
#define RFLR_DIOMAPPING1_DIO2_11 0x0C

#define RFLR_DIOMAPPING1_DIO3_MASK 0xFC
#define RFLR_DIOMAPPING1_DIO3_00 0x00 // Default
#define RFLR_DIOMAPPING1_DIO3_01 0x01
#define RFLR_DIOMAPPING1_DIO3_10 0x02
#define RFLR_DIOMAPPING1_DIO3_11 0x03

/*!
 * RegDioMapping2
 */
#define RFLR_DIOMAPPING2_DIO4_MASK 0x3F
#define RFLR_DIOMAPPING2_DIO4_00 0x00 // Default
#define RFLR_DIOMAPPING2_DIO4_01 0x40
#define RFLR_DIOMAPPING2_DIO4_10 0x80
#define RFLR_DIOMAPPING2_DIO4_11 0xC0

#define RFLR_DIOMAPPING2_DIO5_MASK 0xCF
#define RFLR_DIOMAPPING2_DIO5_00 0x00 // Default
#define RFLR_DIOMAPPING2_DIO5_01 0x10
#define RFLR_DIOMAPPING2_DIO5_10 0x20
#define RFLR_DIOMAPPING2_DIO5_11 0x30

#define RFLR_DIOMAPPING2_MAP_MASK 0xFE
#define RFLR_DIOMAPPING2_MAP_PREAMBLEDETECT 0x01
#define RFLR_DIOMAPPING2_MAP_RSSI 0x00 // Default

/*!
 * RegIrqFlagsMask
 */
#define RFLR_IRQFLAGS_RXTIMEOUT_MASK 0x80
#define RFLR_IRQFLAGS_RXDONE_MASK 0x40
#define RFLR_IRQFLAGS_PAYLOADCRCERROR_MASK 0x20
#define RFLR_IRQFLAGS_VALIDHEADER_MASK 0x10
#define RFLR_IRQFLAGS_TXDONE_MASK 0x08
#define RFLR_IRQFLAGS_CADDONE_MASK 0x04
#define RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL_MASK 0x02
#define RFLR_IRQFLAGS_CADDETECTED_MASK 0x01

/*!
 * RegIrqFlags
 */
#define RFLR_IRQFLAGS_RXTIMEOUT 0x80
#define RFLR_IRQFLAGS_RXDONE 0x40
#define RFLR_IRQFLAGS_PAYLOADCRCERROR 0x20
#define RFLR_IRQFLAGS_VALIDHEADER 0x10
#define RFLR_IRQFLAGS_TXDONE 0x08
#define RFLR_IRQFLAGS_CADDONE 0x04
#define RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL 0x02
#define RFLR_IRQFLAGS_CADDETECTED 0x01

/*!
 * RegPaConfig
 */
#define RF_PACONFIG_PASELECT_MASK 0x7F
#define RF_PACONFIG_PASELECT_PABOOST 0x80
#define RF_PACONFIG_PASELECT_RFO 0x00 // Default

#define RF_PACONFIG_MAX_POWER_MASK 0x8F

#define RF_PACONFIG_OUTPUTPOWER_MASK 0xF0

/*!
 * RegPllHop
 */
#define RFLR_PLLHOP_FASTHOP_MASK 0x7F
#define RFLR_PLLHOP_FASTHOP_ON 0x80
#define RFLR_PLLHOP_FASTHOP_OFF 0x00 // Default

/*!
 * RegPaDac
 */
#define RF_PADAC_20DBM_MASK 0xF8
#define RF_PADAC_20DBM_ON 0x07
#define RF_PADAC_20DBM_OFF 0x04 // Default

#define TIMEOUT_RESET 100
#define FREQ_STEP 61.03515625
#define RF_FREQUENCY 433000000 // Hz
#define RF_MID_BAND_THRESH 525000000

#define RADIO_SX1278_PA_PIN 1 /** 1: use PABOOST 2: use RFO */
#define PABOOST_PIN 1
#define RFO_PIN 2

#define TX_OUTPUT_POWER 20       // dBm
#define LORA_BANDWIDTH 1         // [0: 125 kHz,
                                 //  1: 250 kHz,
                                 //  2: 500 kHz,
                                 //  3: Reserved]
#define LORA_SPREADING_FACTOR 11 // [SF7..SF12]
#define LORA_CODINGRATE 1        // [1: 4/5,
                                 //  2: 4/6,
                                 //  3: 4/7,
                                 //  4: 4/8]
#define LORA_PREAMBLE_LENGTH 8   // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 5    // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON pdFALSE
#define LORA_IQ_INVERSION_ON pdFALSE

#define RX_BUFFER_SIZE 256

#define RX_TIMEOUT_VALUE 10000

/*!
 * Radio driver supported modems
 */
typedef enum
{
    MODEM_FSK = 0,
    MODEM_LORA,
} RadioModems_t;

/*!
 * Radio driver internal state machine states definition
 */
typedef enum
{
    RF_IDLE = 0,
    RF_RX_RUNNING,
    RF_TX_RUNNING,
    RF_CAD,
} RadioState_t;

/*!
 * Radio LoRa modem parameters
 */
typedef struct
{
    uint8_t Power;
    uint32_t Bandwidth;
    uint32_t Datarate;
    uint8_t LowDatarateOptimize;
    uint8_t Coderate;
    uint16_t PreambleLen;
    uint8_t FixLen;
    uint8_t PayloadLen;
    uint8_t CrcOn;
    uint8_t FreqHopOn;
    uint8_t HopPeriod;
    uint8_t IqInverted;
    uint8_t RxContinuous;
    uint32_t TxTimeout;
} RadioLoRaSettings_t;

/*!
 * Radio LoRa packet handler state
 */
typedef struct
{
    int8_t SnrValue;
    int16_t RssiValue;
    uint8_t Size;
} RadioLoRaPacketHandler_t;

/*!
 * Radio Settings
 */
typedef struct
{
    RadioState_t State;
    RadioModems_t Modem;
    uint32_t Channel;
    RadioLoRaSettings_t LoRa;
    RadioLoRaPacketHandler_t LoRaPacketHandler;
} RadioSettings_t;

/*!
 * Radio hardware and global parameters
 */
typedef struct SX1278_s
{
    RadioSettings_t Settings;
} SX1278_t;

/**
 * Perform physical reset on the Lora chip
 */
void lora_reset(void);

/**
 * Sets the radio transceiver in idle mode.
 * Must be used to change registers and access the FIFO.
 */
void lora_idle(void);

/**
 * Sets the radio transceiver in sleep mode.
 * Low power consumption and FIFO is lost.
 */
void lora_sleep(void);

/**
 * Sets the radio transceiver in receive mode.
 * Incoming packets will be received.
 */
void lora_receive(void);

// void lora_receiver(uint32_t timeout);

// void lora_transmitter(uint32_t timeout);

/**
 * Configure power level for transmission
 * @param level 2-17, from least to most power
 */
// void lora_set_tx_power(int level);

/**
 * Set carrier frequency.
 * @param frequency Frequency in Hz
 */
void lora_set_frequency(uint32_t frequency);

// void lora_set_op_mode(uint8_t mode);

void lora_set_tx_config(RadioModems_t modem, int8_t power, uint32_t fdev,
                        uint32_t bandwidth, uint32_t datarate,
                        uint8_t coderate, uint16_t preambleLen,
                        uint8_t fixLen, uint8_t crcOn, uint8_t freqHopOn,
                        uint8_t hopPeriod, uint8_t iqInverted, uint32_t timeout);

void lora_set_rx_config(RadioModems_t modem, uint32_t bandwidth,
                        uint32_t datarate, uint8_t coderate,
                        uint32_t bandwidthAfc, uint16_t preambleLen,
                        uint16_t symbTimeout, uint8_t fixLen,
                        uint8_t payloadLen,
                        uint8_t crcOn, uint8_t freqHopOn, uint8_t hopPeriod,
                        uint8_t iqInverted, uint8_t rxContinuous);

/**
 * Enable appending/verifying packet CRC.
 */
void lora_enable_crc(void);

/**
 * Perform hardware initialization.
 */
uint8_t lora_init(void);

/**
 * Returns non-zero if there is data to read (packet received).
 */
int lora_received(void);

/**
 * Read a received packet.
 * @param buf Buffer for the data.
 * @param size Available size in buffer (bytes).
 * @return Number of bytes received (zero if no packet available).
 */
int lora_receive_packet(uint8_t *buf, int size);

/**
 * Send a packet.
 * @param buf Data to be sent
 * @param size Size of data.
 */
void lora_send_packet(uint8_t *buf, int size);
#endif
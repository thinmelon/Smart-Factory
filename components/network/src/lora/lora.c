#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"
#include <string.h>
#include "lora.h"

static spi_device_handle_t SPI_DEVICE_HANDLE;

// static int __implicit;

/*!
 * Radio hardware and global parameters
 */
SX1278_t SX1278;

/*!
 * Flag used to set the RF switch control pins in low power mode when the radio is not active.
 */
// static uint8_t RADIO_IS_ACTIVE = pdFALSE;

/*!
 * Reception buffer
 */
// static uint8_t RX_TX_BUFFER[RX_BUFFER_SIZE];

/**
 * Write a value to a register.
 * @param reg Register index.
 * @param val Value to write.
 */
static void write_register(int reg, int val)
{
    uint8_t out[2] = {0x80 | reg, val};
    uint8_t in[2];

    spi_transaction_t t = {
        .flags = 0,
        .length = 8 * sizeof(out),
        .tx_buffer = out,
        .rx_buffer = in};

    gpio_set_level(CONFIG_CS_GPIO, 0);
    spi_device_transmit(SPI_DEVICE_HANDLE, &t);
    gpio_set_level(CONFIG_CS_GPIO, 1);

    // printf("Write register[%hX] value: %hX\r\n", reg, val);
}

/**
 * Read the current value of a register.
 * @param reg Register index.
 * @return Value of the register.
 */
static uint8_t read_register(int reg)
{

    uint8_t out[2] = {reg, 0xff};
    uint8_t in[2];

    spi_transaction_t t = {
        .flags = 0,
        .length = 8 * sizeof(out),
        .tx_buffer = out,
        .rx_buffer = in};

    gpio_set_level(CONFIG_CS_GPIO, 0);
    spi_device_transmit(SPI_DEVICE_HANDLE, &t);
    gpio_set_level(CONFIG_CS_GPIO, 1);

    // printf("read_register[%hX] value: %hX\r\n", reg, in[1]);
    return in[1];
}

/*!
 * Performs the Rx chain calibration for LF and HF bands
 * \remark Must be called just after the reset so all registers are at their
 *         default values
 */
// static void rx_chain_calibration(void)
// {
//     uint8_t regPaConfigInitVal;
//     uint32_t initialFreq;

//     // Save context
//     regPaConfigInitVal = read_register(REG_PA_CONFIG);

//     initialFreq = (uint32_t)((double)(((uint32_t)read_register(REG_FRF_MSB) << 16) |
//                                       ((uint32_t)read_register(REG_FRF_MID) << 8) |
//                                       ((uint32_t)read_register(REG_FRF_LSB))) *
//                              (double)FREQ_STEP);
//     printf("[rx_chain_calibration] regPaConfigInitVal - %d initialFreq - %d\r\n", regPaConfigInitVal, initialFreq);

//     // Cut the PA just in case, RFO output, power = -1 dBm
//     write_register(REG_PA_CONFIG, 0x00);

//     // Launch Rx chain calibration for LF band
//     write_register(REG_IMAGECAL, (read_register(REG_IMAGECAL) & RF_IMAGECAL_IMAGECAL_MASK) | RF_IMAGECAL_IMAGECAL_START);
//     while ((read_register(REG_IMAGECAL) & RF_IMAGECAL_IMAGECAL_RUNNING) == RF_IMAGECAL_IMAGECAL_RUNNING)
//     {
//     }

//     // Sets a Frequency in HF band
//     lora_set_frequency(868000000);

//     // Launch Rx chain calibration for HF band
//     write_register(REG_IMAGECAL, (read_register(REG_IMAGECAL) & RF_IMAGECAL_IMAGECAL_MASK) | RF_IMAGECAL_IMAGECAL_START);
//     while ((read_register(REG_IMAGECAL) & RF_IMAGECAL_IMAGECAL_RUNNING) == RF_IMAGECAL_IMAGECAL_RUNNING)
//     {
//     }

//     // Restore context
//     write_register(REG_PA_CONFIG, regPaConfigInitVal);
//     lora_set_frequency(initialFreq);
// }

static uint8_t get_pa_select(void)
{
    if (RADIO_SX1278_PA_PIN == PABOOST_PIN)
    {
        return RF_PACONFIG_PASELECT_PABOOST;
    }
    else if (RADIO_SX1278_PA_PIN == RFO_PIN)
    {
        return RF_PACONFIG_PASELECT_RFO;
    }
    else
    {
        return RF_PACONFIG_PASELECT_PABOOST;
    }
}

void lora_reset(void)
{
    gpio_set_level(CONFIG_RST_GPIO, 0);
    vTaskDelay(pdMS_TO_TICKS(1));
    gpio_set_level(CONFIG_RST_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
}

void lora_idle(void)
{
    write_register(REG_OP_MODE, RFLR_OPMODE_LONGRANGEMODE_ON | RFLR_OPMODE_STANDBY);
}

void lora_sleep(void)
{
    write_register(REG_OP_MODE, RFLR_OPMODE_LONGRANGEMODE_ON | RFLR_OPMODE_SLEEP);
}

void lora_receive(void)
{
    write_register(REG_OP_MODE, RFLR_OPMODE_LONGRANGEMODE_ON | RFLR_OPMODE_RECEIVER);
}

// static void lora_set_antenna_switch(uint8_t rxTx)
// {
//     if (rxTx != 0) // 1: TX, 0: RX
//     {
//         gpio_set_level(CONFIG_RX_ENABLE_GPIO, 0);
//         vTaskDelay(pdMS_TO_TICKS(1));
//         gpio_set_level(CONFIG_TX_ENABLE_GPIO, 1);
//         vTaskDelay(pdMS_TO_TICKS(10));
//     }
//     else
//     {
//         gpio_set_level(CONFIG_TX_ENABLE_GPIO, 0);
//         vTaskDelay(pdMS_TO_TICKS(1));
//         gpio_set_level(CONFIG_RX_ENABLE_GPIO, 1);
//         vTaskDelay(pdMS_TO_TICKS(10));
//     }
// }

// void lora_init_antenna(void)
// {
//     gpio_set_level(CONFIG_RX_ENABLE_GPIO, 1);
//     vTaskDelay(pdMS_TO_TICKS(1));
//     gpio_set_level(CONFIG_TX_ENABLE_GPIO, 0);
//     vTaskDelay(pdMS_TO_TICKS(10));
// }

// void lora_deinit_antenna(void)
// {
//     gpio_set_level(CONFIG_RX_ENABLE_GPIO, 0);
//     vTaskDelay(pdMS_TO_TICKS(1));
//     gpio_set_level(CONFIG_TX_ENABLE_GPIO, 0);
//     vTaskDelay(pdMS_TO_TICKS(10));
// }

// void lora_set_antenna_low_power(uint8_t status)
// {
//     if (RADIO_IS_ACTIVE != status)
//     {
//         RADIO_IS_ACTIVE = status;

//         if (status == pdFALSE)
//         {
//             lora_init_antenna();
//         }
//         else
//         {
//             lora_deinit_antenna();
//         }
//     }
// }

// void lora_set_op_mode(uint8_t mode)
// {
//     if (mode == RFLR_OPMODE_SLEEP)
//     {
//         lora_set_antenna_low_power(pdTRUE);
//     }
//     else
//     {
//         lora_set_antenna_low_power(pdFALSE);
//         if (mode == RFLR_OPMODE_TRANSMITTER)
//         {
//             lora_set_antenna_switch(1);
//         }
//         else
//         {
//             lora_set_antenna_switch(0);
//         }
//     }
//     write_register(REG_OP_MODE, (read_register(REG_OP_MODE) & RFLR_OPMODE_MASK) | mode);
// }

void lora_set_frequency(uint32_t frequency)
{
    SX1278.Settings.Channel = frequency;
    frequency = (uint32_t)((double)frequency / (double)FREQ_STEP);
    write_register(REG_FRF_MSB, (uint8_t)((frequency >> 16) & 0xFF));
    write_register(REG_FRF_MID, (uint8_t)((frequency >> 8) & 0xFF));
    write_register(REG_FRF_LSB, (uint8_t)(frequency & 0xFF));
}

void lora_set_tx_config(RadioModems_t modem, int8_t power, uint32_t fdev,
                        uint32_t bandwidth, uint32_t datarate,
                        uint8_t coderate, uint16_t preambleLen,
                        uint8_t fixLen, uint8_t crcOn, uint8_t freqHopOn,
                        uint8_t hopPeriod, uint8_t iqInverted, uint32_t timeout)
{
    uint8_t paConfig = 0;
    uint8_t paDac = 0;

    // lora_set_modem(modem);

    paConfig = read_register(REG_PA_CONFIG);
    paDac = read_register(REG_LR_PADAC);

    paConfig = (paConfig & RF_PACONFIG_PASELECT_MASK) | get_pa_select();
    paConfig = (paConfig & RF_PACONFIG_MAX_POWER_MASK) | 0x70;

    if ((paConfig & RF_PACONFIG_PASELECT_PABOOST) == RF_PACONFIG_PASELECT_PABOOST)
    {
        if (power > 17)
        {
            paDac = (paDac & RF_PADAC_20DBM_MASK) | RF_PADAC_20DBM_ON;
        }
        else
        {
            paDac = (paDac & RF_PADAC_20DBM_MASK) | RF_PADAC_20DBM_OFF;
        }
        if ((paDac & RF_PADAC_20DBM_ON) == RF_PADAC_20DBM_ON)
        {
            if (power < 5)
            {
                power = 5;
            }
            if (power > 20)
            {
                power = 20;
            }
            paConfig = (paConfig & RF_PACONFIG_OUTPUTPOWER_MASK) | (uint8_t)((uint16_t)(power - 5) & 0x0F);
        }
        else
        {
            if (power < 2)
            {
                power = 2;
            }
            if (power > 17)
            {
                power = 17;
            }
            paConfig = (paConfig & RF_PACONFIG_OUTPUTPOWER_MASK) | (uint8_t)((uint16_t)(power - 2) & 0x0F);
        }
    }
    else
    {
        if (power < -1)
        {
            power = -1;
        }
        if (power > 14)
        {
            power = 14;
        }
        paConfig = (paConfig & RF_PACONFIG_OUTPUTPOWER_MASK) | (uint8_t)((uint16_t)(power + 1) & 0x0F);
    }
    write_register(REG_PA_CONFIG, paConfig);
    write_register(REG_LR_PADAC, paDac);

    SX1278.Settings.LoRa.Power = power;
    /** Fatal error: When using LoRa modem only bandwidths 125, 250 and 500 kHz are supported */
    assert(bandwidth < 3);
    bandwidth += 7;
    SX1278.Settings.LoRa.Bandwidth = bandwidth;
    SX1278.Settings.LoRa.Datarate = datarate;
    SX1278.Settings.LoRa.Coderate = coderate;
    SX1278.Settings.LoRa.PreambleLen = preambleLen;
    SX1278.Settings.LoRa.FixLen = fixLen;
    SX1278.Settings.LoRa.FreqHopOn = freqHopOn;
    SX1278.Settings.LoRa.HopPeriod = hopPeriod;
    SX1278.Settings.LoRa.CrcOn = crcOn;
    SX1278.Settings.LoRa.IqInverted = iqInverted;
    SX1278.Settings.LoRa.TxTimeout = timeout;

    if (datarate > 12)
    {
        datarate = 12;
    }
    else if (datarate < 6)
    {
        datarate = 6;
    }
    if (((bandwidth == 7) && ((datarate == 11) || (datarate == 12))) ||
        ((bandwidth == 8) && (datarate == 12)))
    {
        SX1278.Settings.LoRa.LowDatarateOptimize = (uint8_t)0x01;
    }
    else
    {
        SX1278.Settings.LoRa.LowDatarateOptimize = (uint8_t)0x00;
    }

    if (SX1278.Settings.LoRa.FreqHopOn == pdTRUE)
    {
        write_register(REG_LR_PLLHOP, (read_register(REG_LR_PLLHOP) & RFLR_PLLHOP_FASTHOP_MASK) | RFLR_PLLHOP_FASTHOP_ON);
        write_register(REG_LR_HOPPERIOD, SX1278.Settings.LoRa.HopPeriod);
    }

    write_register(REG_LR_MODEMCONFIG1,
                   (read_register(REG_LR_MODEMCONFIG1) &
                    RFLR_MODEMCONFIG1_BW_MASK &
                    RFLR_MODEMCONFIG1_CODINGRATE_MASK &
                    RFLR_MODEMCONFIG1_IMPLICITHEADER_MASK) |
                       (bandwidth << 4) | (coderate << 1) |
                       fixLen);

    write_register(REG_LR_MODEMCONFIG2,
                   (read_register(REG_LR_MODEMCONFIG2) &
                    RFLR_MODEMCONFIG2_SF_MASK &
                    RFLR_MODEMCONFIG2_RXPAYLOADCRC_MASK) |
                       (datarate << 4) | (crcOn << 2));

    write_register(REG_LR_MODEMCONFIG3,
                   (read_register(REG_LR_MODEMCONFIG3) &
                    RFLR_MODEMCONFIG3_LOWDATARATEOPTIMIZE_MASK) |
                       (SX1278.Settings.LoRa.LowDatarateOptimize << 3));

    write_register(REG_LR_PREAMBLEMSB, (preambleLen >> 8) & 0x00FF);
    write_register(REG_LR_PREAMBLELSB, preambleLen & 0xFF);

    if (datarate == 6)
    {
        write_register(REG_LR_DETECTOPTIMIZE,
                       (read_register(REG_LR_DETECTOPTIMIZE) &
                        RFLR_DETECTIONOPTIMIZE_MASK) |
                           RFLR_DETECTIONOPTIMIZE_SF6);
        write_register(REG_LR_DETECTIONTHRESHOLD,
                       RFLR_DETECTIONTHRESH_SF6);
    }
    else
    {
        write_register(REG_LR_DETECTOPTIMIZE,
                       (read_register(REG_LR_DETECTOPTIMIZE) &
                        RFLR_DETECTIONOPTIMIZE_MASK) |
                           RFLR_DETECTIONOPTIMIZE_SF7_TO_SF12);
        write_register(REG_LR_DETECTIONTHRESHOLD,
                       RFLR_DETECTIONTHRESH_SF7_TO_SF12);
    }
}

void lora_set_rx_config(RadioModems_t modem, uint32_t bandwidth,
                        uint32_t datarate, uint8_t coderate,
                        uint32_t bandwidthAfc, uint16_t preambleLen,
                        uint16_t symbTimeout, uint8_t fixLen,
                        uint8_t payloadLen,
                        uint8_t crcOn, uint8_t freqHopOn, uint8_t hopPeriod,
                        uint8_t iqInverted, uint8_t rxContinuous)
{
    // lora_set_modem(modem);

    /** Fatal error: When using LoRa modem only bandwidths 125, 250 and 500 kHz are supported */
    assert(bandwidth < 3);
    bandwidth += 7;
    SX1278.Settings.LoRa.Bandwidth = bandwidth;
    SX1278.Settings.LoRa.Datarate = datarate;
    SX1278.Settings.LoRa.Coderate = coderate;
    SX1278.Settings.LoRa.PreambleLen = preambleLen;
    SX1278.Settings.LoRa.FixLen = fixLen;
    SX1278.Settings.LoRa.PayloadLen = payloadLen;
    SX1278.Settings.LoRa.CrcOn = crcOn;
    SX1278.Settings.LoRa.FreqHopOn = freqHopOn;
    SX1278.Settings.LoRa.HopPeriod = hopPeriod;
    SX1278.Settings.LoRa.IqInverted = iqInverted;
    SX1278.Settings.LoRa.RxContinuous = rxContinuous;

    if (datarate > 12)
    {
        datarate = 12;
    }
    else if (datarate < 6)
    {
        datarate = 6;
    }

    if (((bandwidth == 7) && ((datarate == 11) || (datarate == 12))) ||
        ((bandwidth == 8) && (datarate == 12)))
    {
        SX1278.Settings.LoRa.LowDatarateOptimize = (uint8_t)0x01;
    }
    else
    {
        SX1278.Settings.LoRa.LowDatarateOptimize = (uint8_t)0x00;
    }

    write_register(REG_LR_MODEMCONFIG1,
                   (read_register(REG_LR_MODEMCONFIG1) &
                    RFLR_MODEMCONFIG1_BW_MASK &
                    RFLR_MODEMCONFIG1_CODINGRATE_MASK &
                    RFLR_MODEMCONFIG1_IMPLICITHEADER_MASK) |
                       (bandwidth << 4) | (coderate << 1) |
                       fixLen);

    write_register(REG_LR_MODEMCONFIG2,
                   (read_register(REG_LR_MODEMCONFIG2) &
                    RFLR_MODEMCONFIG2_SF_MASK &
                    RFLR_MODEMCONFIG2_RXPAYLOADCRC_MASK &
                    RFLR_MODEMCONFIG2_SYMBTIMEOUTMSB_MASK) |
                       (datarate << 4) | (crcOn << 2) |
                       ((symbTimeout >> 8) & ~RFLR_MODEMCONFIG2_SYMBTIMEOUTMSB_MASK));

    write_register(REG_LR_MODEMCONFIG3,
                   (read_register(REG_LR_MODEMCONFIG3) &
                    RFLR_MODEMCONFIG3_LOWDATARATEOPTIMIZE_MASK) |
                       (SX1278.Settings.LoRa.LowDatarateOptimize << 3));
}

void lora_enable_crc(void)
{
    write_register(REG_LR_MODEMCONFIG2, read_register(REG_LR_MODEMCONFIG2) | 0x04);
}

uint8_t lora_init(void)
{
    esp_err_t ret;

    /*
     * Configure CPU hardware to communicate with the radio chip
     */
    gpio_pad_select_gpio(CONFIG_RST_GPIO);
    gpio_set_direction(CONFIG_RST_GPIO, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(CONFIG_CS_GPIO);
    gpio_set_direction(CONFIG_CS_GPIO, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(CONFIG_TX_ENABLE_GPIO);
    gpio_set_direction(CONFIG_TX_ENABLE_GPIO, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(CONFIG_RX_ENABLE_GPIO);
    gpio_set_direction(CONFIG_RX_ENABLE_GPIO, GPIO_MODE_OUTPUT);

    spi_bus_config_t bus = {
        .miso_io_num = CONFIG_MISO_GPIO,
        .mosi_io_num = CONFIG_MOSI_GPIO,
        .sclk_io_num = CONFIG_SCK_GPIO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0};

    ret = spi_bus_initialize(VSPI_HOST, &bus, 0);
    assert(ret == ESP_OK);

    spi_device_interface_config_t dev = {
        .clock_speed_hz = 9000000,
        .mode = 0,
        .spics_io_num = -1,
        .queue_size = 1,
        .flags = 0,
        .pre_cb = NULL};

    ret = spi_bus_add_device(VSPI_HOST, &dev, &SPI_DEVICE_HANDLE);
    assert(ret == ESP_OK);

    /*
     * Perform hardware reset.
     */
    lora_reset();
    // rx_chain_calibration();

    /*
     * Check version.
     */
    uint8_t version;
    uint8_t i = 0;
    while (i++ < TIMEOUT_RESET)
    {
        version = read_register(REG_VERSION);
        if (version == 0x12)
            break;
        vTaskDelay(2);
    }
    assert(i <= TIMEOUT_RESET + 1); // at the end of the loop above, the max value i can reach is TIMEOUT_RESET + 1

    /*
     * Default configuration.
     */
    lora_sleep();

    return 1;
}

/**
 * Returns non-zero if there is data to read (packet received).
 */
int lora_received(void)
{
    if (read_register(REG_LR_IRQFLAGS) & RFLR_IRQFLAGS_RXDONE_MASK)
        return 1;
    return 0;
}

int lora_receive_packet(uint8_t *buf, int size)
{
    int len = 0;

    /*
     * Check interrupts.
     */
    int irq = read_register(REG_LR_IRQFLAGS);
    write_register(REG_LR_IRQFLAGS, irq);
    if ((irq & RFLR_IRQFLAGS_RXDONE_MASK) == 0)
        return 0;
    if (irq & RFLR_IRQFLAGS_PAYLOADCRCERROR_MASK)
        return 0;

    /*
     * Find packet size.
     */
    // if (__implicit)
    //     len = read_register(REG_LR_PAYLOADLENGTH);
    // else
    //     len = read_register(REG_LR_RXNBBYTES);
    len = read_register(REG_LR_RXNBBYTES);

    /*
     * Transfer data from radio.
     */
    lora_idle();
    write_register(REG_LR_FIFO_ADDRESS_PTR, read_register(REG_LR_FIFO_RX_CURRENT_ADDRESS));
    if (len > size)
        len = size;
    for (int i = 0; i < len; i++)
        *buf++ = read_register(REG_FIFO);

    return len;
}

/**
 * Send a packet.
 * @param buf Data to be sent
 * @param size Size of data.
 */
void lora_send_packet(uint8_t *buf, int size)
{
    /*
     * Transfer data to radio.
     */
    lora_idle();
    // Full buffer used for Tx
    write_register(REG_LR_FIFO_TX_BASE_ADDRESS, 0);
    write_register(REG_LR_FIFO_ADDRESS_PTR, 0);

    for (int i = 0; i < size; i++)
        write_register(REG_FIFO, *buf++);

    // Initializes the payload size
    write_register(REG_LR_PAYLOADLENGTH, size);

    /*
     * Start transmission and wait for conclusion.
     */
    write_register(REG_OP_MODE, RFLR_OPMODE_LONGRANGEMODE_ON | RFLR_OPMODE_TRANSMITTER);
    while ((read_register(REG_LR_IRQFLAGS) & RFLR_IRQFLAGS_TXDONE) == 0)
        vTaskDelay(2);

    write_register(REG_LR_IRQFLAGS, RFLR_IRQFLAGS_TXDONE);
}
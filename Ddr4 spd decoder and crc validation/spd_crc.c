#include "spd_crc.h"

#define SPD_BASE_DATA_LENGTH      126U
#define SPD_BASE_CRC_LSB_INDEX    126U
#define SPD_BASE_CRC_MSB_INDEX    127U

#define SPD_BLOCK1_START          128U
#define SPD_BLOCK1_DATA_LENGTH    126U
#define SPD_BLOCK1_CRC_LSB_INDEX  254U
#define SPD_BLOCK1_CRC_MSB_INDEX  255U

uint16_t SPD_Crc16(const uint8_t *data, size_t length) 
{
    uint16_t crc = 0x0000U; 

    if (data == NULL)
    {
        return 0U;
    }

    for (size_t index = 0U; index < length; ++index)
    {
        crc ^= ((uint16_t)data[index] << 8U);

        for (uint8_t bit = 0U; bit < 8U; ++bit)
        {
            if ((crc & 0x8000U) != 0U)
            {
                crc = (uint16_t)((crc << 1U) ^ 0x1021U);
            }
            else
            {
                crc = (uint16_t)(crc << 1U);
            }
        }
    }

    return crc;
}

bool SPD_VerifyBaseCrc(
    const uint8_t *spd,
    size_t size,
    uint16_t *calculated,
    uint16_t *stored)
{
    uint16_t calculated_value;
    uint16_t stored_value;

    if ((spd == NULL) || (size < 128U))
    {
        return false; 
    }

    calculated_value = SPD_Crc16(spd, SPD_BASE_DATA_LENGTH); 

    stored_value =
        (uint16_t)spd[SPD_BASE_CRC_LSB_INDEX] |
        ((uint16_t)spd[SPD_BASE_CRC_MSB_INDEX] << 8U);

    if (calculated != NULL)
    {
        *calculated = calculated_value;
    }

    if (stored != NULL)
    {
        *stored = stored_value;
    }

    return calculated_value == stored_value;
}  

bool SPD_VerifyBlock1Crc(
    const uint8_t *spd,
    size_t size,
    uint16_t *calculated,
    uint16_t *stored)
{
    uint16_t calculated_value;
    uint16_t stored_value;

    if ((spd == NULL) || (size < 256U))
    {
        return false;
    }

    calculated_value = SPD_Crc16(&spd[SPD_BLOCK1_START], SPD_BLOCK1_DATA_LENGTH);

    stored_value =
        (uint16_t)spd[SPD_BLOCK1_CRC_LSB_INDEX] |
        ((uint16_t)spd[SPD_BLOCK1_CRC_MSB_INDEX] << 8U); 

    if (calculated != NULL)
    {
        *calculated = calculated_value;
    }

    if (stored != NULL)
    {
        *stored = stored_value;
    }

    return calculated_value == stored_value;
}      
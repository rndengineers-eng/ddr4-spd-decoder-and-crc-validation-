 #include "spd_crc.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define SPD_SIZE 512U

static void WriteBaseCrc(uint8_t *spd)
{
    uint16_t crc;

    crc = SPD_Crc16(spd, 126U);

    spd[126] = (uint8_t)(crc & 0xFFU);
    spd[127] = (uint8_t)((crc >> 8U) & 0xFFU);
}

static void WriteBlock1Crc(uint8_t *spd)
{
    uint16_t crc;

    crc = SPD_Crc16(&spd[128], 126U);

    spd[254] = (uint8_t)(crc & 0xFFU);
    spd[255] = (uint8_t)((crc >> 8U) & 0xFFU);
}

int main(void)
{
    uint8_t spd[SPD_SIZE];
    uint16_t calculated;
    uint16_t stored;

    memset(spd, 0, sizeof(spd));

    WriteBaseCrc(spd);
    WriteBlock1Crc(spd);

    if (!SPD_VerifyBaseCrc(spd, sizeof(spd), &calculated, &stored)) 
    {
        printf("Base CRC test: FAIL\n");
        return 1;
    }

    if (!SPD_VerifyBlock1Crc(spd, sizeof(spd), &calculated, &stored)) 
    {
        printf("Block1 CRC test: FAIL\n");
        return 1;
    }

    spd[4] ^= 0x01U;

    if (SPD_VerifyBaseCrc(spd, sizeof(spd), NULL, NULL))
    {
        printf("Corruption detection: FAIL\n");
        return 1;
    }

    printf("CRC tests: PASS\n");

    return 0;
}     
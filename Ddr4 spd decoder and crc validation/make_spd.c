#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define SPD_SIZE 512U 

static uint16_t Crc16(const uint8_t *data, size_t length)
{
    uint16_t crc = 0x0000U;

    for (size_t i = 0U; i < length; ++i)
    {
        crc ^= (uint16_t)((uint16_t)data[i] << 8U);

        for (int bit = 0; bit < 8; ++bit)
        {
            crc = (crc & 0x8000U) ? (uint16_t)((crc << 1U) ^ 0x1021U)
                      : (uint16_t)(crc << 1U); 
        }
    } 

    return crc;
} 

int main(int argc, char *argv[])
{
    const char *filename = (argc > 1) ? argv[1] : "spd.bin";

    uint8_t spd[SPD_SIZE];
    uint16_t crc;
    FILE *file; 

    memset(spd, 0, sizeof(spd));

    spd[0] = 0x23;      /* 512-byte device, 384 bytes used */
    spd[1] = 0x12;      /* SPD revision 1.2 */
    spd[2] = 0x0C;      /* DDR4 SDRAM */
    spd[3] = 0x03;      /* SO-DIMM */
    spd[4] = 0x06;      /* 16 Gb density */

    spd[12] = 0x01;     /* x8 devices, 1 rank */
    spd[13] = 0x03;     /* 64-bit primary bus */

    spd[17] = 0x00;     /* MTB = 125 ps, FTB = 1 ps */
    spd[18] = 0x05;     /* tCKAVGmin = 5 * 125 = 625 ps */

    spd[125] = 0x00;    /* Fine timing offset */

    crc = Crc16(&spd[0], 126U);

    spd[126] = (uint8_t)(crc & 0xFFU);
    spd[127] = (uint8_t)((crc >> 8U) & 0xFFU);

    crc = Crc16(&spd[128], 126U); 

    spd[254] = (uint8_t)(crc & 0xFFU);
    spd[255] = (uint8_t)((crc >> 8U) & 0xFFU);

    file = fopen(filename, "wb"); 

    if (file == NULL)
    {
        printf("ERROR: cannot create %s\n", filename);
        return 1;
    }

    if (fwrite(spd, 1U, SPD_SIZE, file) != SPD_SIZE)
    {
        printf("ERROR: write failed\n");

        fclose(file);
        return 1;
    }

    fclose(file);

    printf("%s (%u bytes)\n", filename, SPD_SIZE);

    return 0;
}  
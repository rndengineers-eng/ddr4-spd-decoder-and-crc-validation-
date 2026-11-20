#include "spd_crc.h"
#include "spd_decoder.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void GenerateValidTestSpd(SPD_Image *image)  
{
    uint16_t crc;

    memset(image, 0, sizeof(*image));  

    image->size = SPD_IMAGE_SIZE; 

    image->data[0] = 0x23U; 

    image->data[1] = 0x12U; 

    image->data[2] = 0x0CU;

    image->data[3] = 0x03U; 

    image->data[4] = 0x06U; 

    image->data[11] = 0x03U;

    image->data[12] = 0x01U; 
    
    image->data[13] = 0x03U;

    image->data[15] = 0x00U;

    image->data[16] = 0x00U;

    image->data[18] = 5U; 

    image->data[125] = 0x00U;

    crc = SPD_Crc16(image->data, 126U); 

    image->data[126] = (uint8_t) (crc & 0xFFU);

    image->data[127] = (uint8_t) (crc >> 8U);

    crc = SPD_Crc16(&image->data[128], 126U);  

    image->data[254] = (uint8_t) (crc & 0xFFU);

    image->data[255] =  (uint8_t) (crc >> 8U); 
}

int main(void)
{
    SPD_Image image;
    SPD_DecodedInfo info;

    GenerateValidTestSpd(&image); 

    if (!SPD_Decode(&image, &info))
    {
        printf("SPD_Decode: FAIL\n"); 
        return 1; 
    }

    if (info.memory_type != SPD_MEMORY_TYPE_DDR4)
    {
        printf("Memory type test: FAIL\n"); 
        return 1;
    }

    if (info.module_type != SPD_MODULE_TYPE_SODIMM)
    {
        printf("Module type test: FAIL\n"); 
        return 1;
    } 

    if (info.dram_density_mbit != 16384U)
    {
        printf("Density test: FAIL\n"); 
        return 1;
    } 

    if (info.device_width_bits != 8U) 
    {
        printf("Device width test: FAIL\n"); 
        return 1;
    }

    if (info.rank_count != 1U) 
    {
        printf("Rank test: FAIL\n"); 
        return 1;
    } 

    if (info.primary_bus_width_bits != 64U)
    {
        printf("Bus width test: FAIL\n"); 
        return 1;
    } 

    if (info.calculated_capacity_mib != 16384U) 
    {
        printf( "Capacity test: FAIL\n"); 
        return 1;
    } 

    if (info.tckavg_min_ps != 625)
    {
        printf("tCK test: FAIL\n"); 
        return 1;
    }

    if (info.maximum_data_rate_mts != 3200U) 
    {
        printf("Data-rate test: FAIL\n"); 
        return 1;
    } 

    if (!info.base_crc_valid)
    {
        printf("Base CRC test: FAIL\n"); 
        return 1;
    }

    if (!info.block1_crc_valid)
    {
        printf("Block1 CRC test: FAIL\n"); 
        return 1;
    } 

    printf("Decoder tests: PASS\n"); 
    return 0;
}     
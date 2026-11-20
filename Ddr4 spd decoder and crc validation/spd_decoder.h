#ifndef SPD_DECODER_H
#define SPD_DECODER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h> 

#define SPD_IMAGE_SIZE 512U  

#define SPD_BYTE_DEVICE_INFO       0U
#define SPD_BYTE_REVISION          1U
#define SPD_BYTE_MEMORY_TYPE       2U
#define SPD_BYTE_MODULE_TYPE       3U
#define SPD_BYTE_DENSITY_BANKS     4U
#define SPD_BYTE_VOLTAGE          11U
#define SPD_BYTE_ORGANIZATION     12U
#define SPD_BYTE_BUS_WIDTH        13U
#define SPD_BYTE_EXT_MODULE_TYPE  15U
#define SPD_BYTE_RESERVED_16      16U
#define SPD_BYTE_TIMEBASES        17U
#define SPD_BYTE_TCKAVG_MIN       18U

#define SPD_BYTE_TCK_FINE        125U

#define SPD_MEMORY_TYPE_DDR4     0x0CU
#define SPD_MODULE_TYPE_SODIMM   0x03U

#define TARGET_CAPACITY_MIB       16384U
#define TARGET_SPEED_MTS           3200U
#define TARGET_DENSITY_MBIT       16384U
#define TARGET_DEVICE_WIDTH_BITS      8U
#define TARGET_RANK_COUNT             1U
#define TARGET_BUS_WIDTH_BITS        64U
#define TARGET_TCK_PS               625

typedef struct
{
    uint8_t data[SPD_IMAGE_SIZE];
    size_t size;

} SPD_Image;

typedef struct
{
    uint8_t revision_major;
    uint8_t revision_minor;

    uint8_t memory_type;
    uint8_t module_type;

    uint32_t dram_density_mbit;

    uint8_t device_width_bits;
    uint8_t rank_count; 
    uint8_t primary_bus_width_bits;

    uint32_t calculated_capacity_mib;

    int32_t tckavg_min_ps;
    uint32_t maximum_data_rate_mts;

    uint16_t base_crc_calculated;
    uint16_t base_crc_stored;
    bool base_crc_valid;

    uint16_t block1_crc_calculated;
    uint16_t block1_crc_stored;
    bool block1_crc_valid;

    bool decoded;

} SPD_DecodedInfo;

bool SPD_LoadBinary(const char *filename,SPD_Image *image);
 
bool SPD_Decode(const SPD_Image *image,SPD_DecodedInfo *info);

void SPD_PrintDecodedInfo(const SPD_DecodedInfo *info);  

#endif        
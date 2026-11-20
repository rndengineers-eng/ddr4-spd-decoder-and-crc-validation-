#include "spd_decoder.h"
#include "spd_crc.h"

#include <stdio.h>
#include <string.h>  

static bool DecodeDensity(uint8_t code, uint32_t *density_mbit)
{
    if ((density_mbit == NULL) || (code > 7U))
    {
        return false;  
    }

    *density_mbit = 256U << code;   /* 0=256Mb ... 6=16Gb, 7=32Gb */

    return true;
}

static bool DecodeDeviceWidth(uint8_t code, uint8_t *width_bits)
{
    if ((width_bits == NULL) || (code > 3U))
    {
        return false;
    }

    *width_bits = (uint8_t)(4U << code);   /* x4, x8, x16, x32 */

    return true;
}

static bool DecodePrimaryBusWidth(uint8_t code, uint8_t *width_bits)
{
    if ((width_bits == NULL) || (code > 3U))
    {
        return false;
    }

    *width_bits = (uint8_t)(8U << code);   /* 8, 16, 32, 64 */

    return true;
}

static uint32_t CalculateModuleCapacityMiB(
    uint32_t density_mbit,
    uint8_t primary_bus_width,
    uint8_t device_width,
    uint8_t rank_count)
{
    uint32_t device_capacity_mib;
    uint32_t devices_per_rank;

    if ((density_mbit == 0U) ||
        (primary_bus_width == 0U) ||
        (device_width == 0U) ||
        (rank_count == 0U))
    {
        return 0U;
    }

    if ((primary_bus_width % device_width) != 0U)
    {
        return 0U;
    }

    device_capacity_mib = density_mbit / 8U;

    devices_per_rank = (uint32_t)(primary_bus_width / device_width);

    return device_capacity_mib * devices_per_rank * rank_count; 
}

static bool DecodeTck(const SPD_Image *image, int32_t *tck_ps,
    uint32_t *data_rate_mts) 
{
    int32_t mtb_value;
    int32_t ftb_value;
    int32_t result_ps;

    if ((image == NULL) ||
        (tck_ps == NULL) ||
        (data_rate_mts == NULL))
    {
        return false;
    }

    /* Byte 17 = 0x00 -> MTB = 125 ps, FTB = 1 ps */
    if (image->data[SPD_BYTE_TIMEBASES] != 0x00U)
    {
        return false;
    }

    if (image->data[SPD_BYTE_TCKAVG_MIN] == 0U)
    {
        return false;
    }

    mtb_value = (int32_t)image->data[SPD_BYTE_TCKAVG_MIN] * 125;

    ftb_value = (int32_t)(int8_t)image->data[SPD_BYTE_TCK_FINE]; 

    result_ps = mtb_value + ftb_value;

    if (result_ps <= 0)
    { 
        return false;
    }

    *tck_ps = result_ps;

    *data_rate_mts = (uint32_t)((2000000L + (result_ps / 2)) / result_ps); 

    return true;
}

bool SPD_LoadBinary(const char *filename, SPD_Image *image)
{
    FILE *file;
    size_t bytes_read;
    int extra_byte;

    if ((filename == NULL) || (image == NULL))
    {
        return false;
    }

    memset(image, 0, sizeof(*image));

    file = fopen(filename, "rb");

    if (file == NULL)
    {
        return false;
    }

    bytes_read = fread(image->data, 1U, SPD_IMAGE_SIZE, file);

    image->size = bytes_read;

    if (bytes_read != SPD_IMAGE_SIZE)
    {
        fclose(file);

        return false;
    }

    extra_byte = fgetc(file);   /* file must be exactly 512 bytes */

    fclose(file);

    return (extra_byte == EOF);
}

bool SPD_Decode(const SPD_Image *image, SPD_DecodedInfo *info)
{
    uint8_t density_code;
    uint8_t width_code;
    uint8_t bus_width_code;

    if ((image == NULL) ||
        (info == NULL) ||
        (image->size != SPD_IMAGE_SIZE))
    {
        return false;
    }

    memset(info, 0, sizeof(*info));

    info->revision_major = (uint8_t)((image->data[SPD_BYTE_REVISION] >> 4U) & 0x0FU);

    info->revision_minor = (uint8_t)(image->data[SPD_BYTE_REVISION] & 0x0FU);

    info->memory_type = image->data[SPD_BYTE_MEMORY_TYPE];

    info->module_type = (uint8_t)(image->data[SPD_BYTE_MODULE_TYPE] & 0x0FU);

    /* An invalid field no longer aborts decoding: the value stays 0, it is
       still printed, and the validator reports it as an ERROR. */
    density_code = (uint8_t)(image->data[SPD_BYTE_DENSITY_BANKS] & 0x0FU); 

    (void)DecodeDensity(density_code, &info->dram_density_mbit); 

    width_code = (uint8_t)(image->data[SPD_BYTE_ORGANIZATION] & 0x07U);

    (void)DecodeDeviceWidth(width_code, &info->device_width_bits); 

    info->rank_count = (uint8_t) (((image->data[SPD_BYTE_ORGANIZATION] >> 3U) & 0x07U) + 1U);

    bus_width_code = (uint8_t)(image->data[SPD_BYTE_BUS_WIDTH] & 0x07U); 

    (void)DecodePrimaryBusWidth(bus_width_code, &info->primary_bus_width_bits); 

    info->calculated_capacity_mib =CalculateModuleCapacityMiB(
            info->dram_density_mbit,
            info->primary_bus_width_bits,
            info->device_width_bits,
            info->rank_count); 

    (void)DecodeTck(image, &info->tckavg_min_ps, &info->maximum_data_rate_mts);

    info->base_crc_valid = SPD_VerifyBaseCrc(image->data, image->size,
            &info->base_crc_calculated, &info->base_crc_stored);

    info->block1_crc_valid = SPD_VerifyBlock1Crc(image->data, image->size,
            &info->block1_crc_calculated, &info->block1_crc_stored);

    info->decoded = true;

    return true; 
}

void SPD_PrintDecodedInfo(const SPD_DecodedInfo *info)
{
    if ((info == NULL) || (!info->decoded))
    {
        printf("ERROR: SPD data has not been decoded.\n");

        return;
    }

    printf("\n");
    printf("----------------------------------------\n");
    printf(" DDR4 SPD DECODED INFORMATION\n");
    printf("----------------------------------------\n");

    printf("SPD Revision          : %u.%u\n", (unsigned)info->revision_major, (unsigned)info->revision_minor);

    printf("Memory Type           : 0x%02X", (unsigned)info->memory_type);  

    if (info->memory_type == SPD_MEMORY_TYPE_DDR4)
    {
        printf(" (DDR4)");
    }

    printf("\n");

    printf("Module Type           : 0x%02X", (unsigned)info->module_type); 

    if (info->module_type == SPD_MODULE_TYPE_SODIMM)
    {
        printf(" (SO-DIMM)");
    }

    printf("\n");

    printf("DRAM Density          : %lu Mbit (%lu Gb)\n", (unsigned long)info->dram_density_mbit, (unsigned long)(info->dram_density_mbit / 1024U));

    printf("SDRAM Device Width    : x%u\n", (unsigned)info->device_width_bits);

    printf("Rank Count            : %u\n", (unsigned)info->rank_count);

    printf("Primary Bus Width     : %u-bit\n", (unsigned)info->primary_bus_width_bits);

    printf("Calculated Capacity   : %lu MiB\n", (unsigned long)info->calculated_capacity_mib);

    printf("tCKAVGmin             : %ld ps\n", (long)info->tckavg_min_ps); 

    printf("Maximum Data Rate     : %lu MT/s\n", (unsigned long)info->maximum_data_rate_mts);

    printf("\n");
    printf("Base CRC\n");
    printf("  Stored              : 0x%04X\n", (unsigned)info->base_crc_stored);
    printf("  Calculated          : 0x%04X\n", (unsigned)info->base_crc_calculated);
    printf("  Result              : %s\n", info->base_crc_valid ? "PASS" : "FAIL");

    printf("\n");
    printf("Block-1 CRC\n");
    printf("  Stored              : 0x%04X\n", (unsigned)info->block1_crc_stored);
    printf("  Calculated          : 0x%04X\n", (unsigned)info->block1_crc_calculated);
    printf("  Result              : %s\n", info->block1_crc_valid ? "PASS" : "FAIL");
}   
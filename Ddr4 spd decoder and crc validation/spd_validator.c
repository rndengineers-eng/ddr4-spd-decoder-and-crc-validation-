#include "spd_validator.h"

#include <stdio.h>
#include <string.h>

static void ReportResult(
    SPD_ValidationStatus status,
    const char *field,
    const char *message,
    SPD_ValidationSummary *summary)  
    
    {
        const char *status_string;
        
        if ((field == NULL) ||
        (message == NULL) ||
        (summary == NULL))
        
        {
            return;
        }
        
        switch (status)
        {
            case SPD_STATUS_PASS:
            status_string = "PASS";
            summary->pass_count++;
            break;

        case SPD_STATUS_WARNING:
            status_string = "WARNING";
            summary->warning_count++;
            break;

        case SPD_STATUS_ERROR:
            status_string = "ERROR";
            summary->error_count++;
            break;

        case SPD_STATUS_NOT_CHECKED:
        default:
            status_string = "NOT CHECKED";
            summary->not_checked_count++;
            break;
    } 

    printf("%-12s %-24s %s\n", status_string, field, message); 
} 

static void ValidateReservedBytes(
    const SPD_Image *image, SPD_ValidationSummary *summary) 
{
    bool valid = true;

    if ((image == NULL) ||
        (summary == NULL))
    {
        return;
    }

    for (size_t index = 132U; index <= 191U; ++index)
    {
        if (image->data[index] != 0x00U)
        { 
            printf(
                "ERROR        Reserved byte            "
                "Byte %zu = 0x%02X, expected 0x00\n", index, image->data[index]); 

            summary->error_count++;
            valid = false;
        }
    }

    if (valid)
    {
        ReportResult(SPD_STATUS_PASS, "Reserved 132-191",
            "Reserved bytes are zero", summary); 
    }
}

SPD_ValidationStatus SPD_Validate(
    const SPD_Image *image,
    const SPD_DecodedInfo *info,
    SPD_ValidationSummary *summary
)
{
    uint8_t total_bytes_code;

    if ((image == NULL) ||
        (info == NULL) ||
        (summary == NULL))
    {
        return SPD_STATUS_ERROR;
    }

    memset(summary, 0, sizeof(*summary)); 

    printf("\n");
    printf("---------------------------------------\n");
    printf(" DDR4 SPD VALIDATION REPORT\n");
    printf("---------------------------------------\n");

    if (image->size == SPD_IMAGE_SIZE)
    {
        ReportResult(SPD_STATUS_PASS, "File Size",
            "512-byte SPD image", summary); 
    }
    else
    {
        ReportResult(SPD_STATUS_ERROR, "File Size",
            "Expected exactly 512 bytes", summary);
    }

    if (!info->decoded)
    {
        ReportResult(SPD_STATUS_ERROR, "Decoder",
            "SPD decoding was unsuccessful", summary); 

        return SPD_STATUS_ERROR;
    }

    total_bytes_code = (uint8_t)
        ((image->data[SPD_BYTE_DEVICE_INFO] >> 4U) & 0x07U); 

    if (total_bytes_code == 2U)
    {
        ReportResult(SPD_STATUS_PASS, "SPD Device Size",
            "Byte 0 indicates 512 bytes", summary); 
    }
    else
    {
        ReportResult(SPD_STATUS_ERROR, "SPD Device Size",
            "Byte 0 does not indicate 512 bytes", summary); 
    }

    if ((image->data[0] & 0x80U) == 0U)
    {
        ReportResult(SPD_STATUS_PASS, "Byte 0 Bit 7",
            "Reserved bit is zero", summary); 
    }
    else
    {
        ReportResult(SPD_STATUS_ERROR, "Byte 0 Bit 7",
            "Reserved bit must be zero", summary); 
    }

    if (info->revision_major == 1U)
    {
        ReportResult(SPD_STATUS_PASS, "SPD Revision",
            "Supported DDR4 SPD major revision", summary); 
    }
    else
    {
        ReportResult(SPD_STATUS_ERROR, "SPD Revision",
            "Unsupported SPD major revision", summary); 
    }

    if (info->memory_type == SPD_MEMORY_TYPE_DDR4)
    {
        ReportResult(SPD_STATUS_PASS, "Byte 2 Memory Type",
            "DDR4 = 0x0C", summary); 
    }
    else
    {
        ReportResult(SPD_STATUS_ERROR, "Byte 2 Memory Type",
            "Expected DDR4 value 0x0C", summary); 
    }

    if (info->module_type == SPD_MODULE_TYPE_SODIMM)
    {
        ReportResult(SPD_STATUS_PASS, "Byte 3 Module Type",
            "SO-DIMM = 0x03", summary); 
    }
    else
    {
        ReportResult(SPD_STATUS_ERROR, "Byte 3 Module Type",
            "Expected SO-DIMM", summary); 
    }

    if (info->dram_density_mbit == TARGET_DENSITY_MBIT)
    {
        ReportResult(SPD_STATUS_PASS, "Byte 4 Density",
            "16Gb SDRAM devices", summary); 
    }
    else
    {
        ReportResult(SPD_STATUS_ERROR, "Byte 4 Density",
            "Expected 16Gb SDRAM device", summary); 
    }

    if (info->device_width_bits == TARGET_DEVICE_WIDTH_BITS)
    {
        ReportResult(SPD_STATUS_PASS, "Byte 12 Width",
            "x8 SDRAM organization", summary); 
    }
    else
    {
        ReportResult(SPD_STATUS_ERROR, "Byte 12 Width",
            "Expected x8 SDRAM device", summary); 
    }

    if (info->rank_count == TARGET_RANK_COUNT)
    {
        ReportResult(SPD_STATUS_PASS, "Byte 12 Ranks",
            "Single-rank configuration", summary); 
    }
    else
    {
        ReportResult(SPD_STATUS_ERROR, "Byte 12 Ranks",
            "Expected single rank", summary);
    }

    if (info->primary_bus_width_bits == TARGET_BUS_WIDTH_BITS)
    {
        ReportResult(SPD_STATUS_PASS, "Byte 13 Bus Width",
            "64-bit primary bus", summary); 
    }
    else
    {
        ReportResult(SPD_STATUS_ERROR, "Byte 13 Bus Width",
            "Expected 64-bit primary bus", summary); 
    }

    if (info->calculated_capacity_mib == TARGET_CAPACITY_MIB)
    {
        ReportResult(SPD_STATUS_PASS, "Module Capacity",
            "16384 MiB / 16GB target", summary);
    }
    else
    {
        ReportResult(SPD_STATUS_ERROR, "Module Capacity",
            "Expected 16384 MiB", summary); 
    }

    if (image->data[SPD_BYTE_TIMEBASES] == 0x00U)
    {
        ReportResult(SPD_STATUS_PASS, "Byte 17 Timebase",
            "MTB=125ps, FTB=1ps", summary); 
    }
    else
    {
        ReportResult(SPD_STATUS_ERROR, "Byte 17 Timebase",
            "Unsupported/reserved timebase encoding", summary); 
    }

    if (info->tckavg_min_ps == TARGET_TCK_PS)
    {
        ReportResult(SPD_STATUS_PASS, "tCKAVGmin", "625 ps", summary); 
    }
    else
    {
        ReportResult(SPD_STATUS_ERROR, "tCKAVGmin",
            "DDR4-3200 target expects 625 ps", summary); 
    }

    if (info->maximum_data_rate_mts == TARGET_SPEED_MTS)
    {
        ReportResult(SPD_STATUS_PASS, "Data Rate",
            "DDR4-3200", summary); 
    }
    else
    {
        ReportResult(SPD_STATUS_ERROR, "Data Rate",
            "Expected 3200 MT/s", summary); 
    }

    if (image->data[SPD_BYTE_EXT_MODULE_TYPE] == 0x00U)
    {
        ReportResult(SPD_STATUS_PASS, "Byte 15",
            "Extended module type unused", summary); 
    }
    else
    {
        ReportResult(SPD_STATUS_WARNING, "Byte 15",
            "Unexpected extended module type", summary);
    }

    if (image->data[SPD_BYTE_RESERVED_16] == 0x00U)
    {
        ReportResult(SPD_STATUS_PASS, "Byte 16",
            "Reserved byte is zero", summary); 
    }
    else
    {
        ReportResult(SPD_STATUS_ERROR, "Byte 16", "Reserved byte must be 0x00", summary); 
    }

    if (info->base_crc_valid)
    {
        ReportResult(SPD_STATUS_PASS,"Bytes 126-127 CRC",
            "Base configuration CRC valid", summary); 
    } 
    else
    {
        ReportResult(SPD_STATUS_ERROR,"Bytes 126-127 CRC",
            "Base configuration CRC mismatch", summary); 
    }

    if (info->block1_crc_valid)
    {
        ReportResult(SPD_STATUS_PASS, "Bytes 254-255 CRC",
            "Module-specific block CRC valid", summary); 
    }
    else
    {
        ReportResult(SPD_STATUS_ERROR, "Bytes 254-255 CRC",
            "Module-specific block CRC mismatch", summary); 
    }

    ValidateReservedBytes(image, summary); 

    ReportResult(
        SPD_STATUS_NOT_CHECKED, "Full Timing Set",
        "tAA/tRCD/tRP/tRAS/tRC validation pending rule table",summary); 

    ReportResult(SPD_STATUS_NOT_CHECKED, "Manufacturing Data",
        "Manufacturer/serial/part fields not validated here", summary);

    if (summary->error_count > 0U)
    {
        return SPD_STATUS_ERROR;
    }

    if (summary->warning_count > 0U)
    {
        return SPD_STATUS_WARNING;
    }

    return SPD_STATUS_PASS;
}

void SPD_PrintValidationSummary(const SPD_ValidationSummary *summary) 
{
    if (summary == NULL)
    {
        return;
    }

    printf("\n");
    printf("---------------------------------------\n");
    printf(" VALIDATION SUMMARY\n");
    printf("---------------------------------------\n");

    printf("PASS        : %u\n", summary->pass_count); 

    printf("WARNING     : %u\n", summary->warning_count);

    printf("ERROR       : %u\n", summary->error_count); 

    printf("NOT CHECKED : %u\n", summary->not_checked_count); 
}       
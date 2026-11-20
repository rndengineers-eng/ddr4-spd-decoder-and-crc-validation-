#include "spd_decoder.h"
#include "spd_validator.h"

#include <stdio.h>  

int main(int argc, char *argv[])
{
    SPD_Image image;
    SPD_DecodedInfo decoded;
    SPD_ValidationSummary summary;
    SPD_ValidationStatus status;

    printf("----------------------------------------\n"); 
    printf("Q TECHNOLOGIES\n");
    printf("DDR4-3200 16GB SO-DIMM\n");
    printf("SPD DECODER / CRC / VALIDATOR\n");
    printf("----------------------------------------\n");

    if (argc != 2)
    {
        printf("\nPlease provide a 512-byte SPD binary file.\n");
        printf("Example: %s spd.bin\n", argv[0]);

        return 1;
    }

    printf("\nInput SPD file: %s\n", argv[1]);

    if (!SPD_LoadBinary(argv[1], &image))
    {
        printf("SPD binary load : FAIL\n");
        printf("ERROR: Could not open the file or the file size is not exactly 512 bytes.\n");

        return 1; 
    }

    printf("SPD binary load : PASS\n");

    if (!SPD_Decode(&image, &decoded))
    {
        printf("\nERROR: SPD decoding failed.\n");

        return 1;
    }

    SPD_PrintDecodedInfo(&decoded);

    status = SPD_Validate(&image, &decoded, &summary); 

    SPD_PrintValidationSummary(&summary);

    printf("\n----------------------------------------\n");
    printf("FINAL RESULT\n");
    printf("----------------------------------------\n");

    switch (status)
    {
        case SPD_STATUS_PASS:
            printf("SPD VALIDATION : PASS\n");
            return 0;

        case SPD_STATUS_WARNING:
            printf("SPD VALIDATION : PASS WITH WARNING\n");
            return 0;

        case SPD_STATUS_ERROR: 
            printf("SPD VALIDATION : FAIL\n");
            return 2;

        case SPD_STATUS_NOT_CHECKED:
            printf("SPD VALIDATION : NOT CHECKED\n");
            return 3;

        default:
            printf("SPD VALIDATION : UNKNOWN STATUS\n");
            return 4;
    }
}       
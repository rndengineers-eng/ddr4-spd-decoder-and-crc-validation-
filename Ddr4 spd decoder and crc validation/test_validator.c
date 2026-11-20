#include "spd_crc.h"
#include "spd_decoder.h"
#include "spd_validator.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void CreateValidImage(SPD_Image *image)
{
    uint16_t crc;

    memset(image, 0, sizeof(*image)); 

    image->size = SPD_IMAGE_SIZE;  

    image->data[0]  = 0x23U;
    image->data[1]  = 0x12U; 
    image->data[2]  = 0x0CU;
    image->data[3]  = 0x03U;
    image->data[4]  = 0x06U;
    image->data[11] = 0x03U;
    image->data[12] = 0x01U;
    image->data[13] = 0x03U;
    image->data[15] = 0x00U;
    image->data[16] = 0x00U;
    image->data[17] = 0x00U;
    image->data[18] = 5U; 

    image->data[125] = 0x00U;

    crc = SPD_Crc16(image->data, 126U); 

    image->data[126] = (uint8_t) (crc & 0xFFU);

    image->data[127] = (uint8_t) (crc >> 8U); 

    crc = SPD_Crc16(&image->data[128], 126U); 

    image->data[254] = (uint8_t) (crc & 0xFFU);

    image->data[255] = (uint8_t) (crc >> 8U);
} 

int main(void)
{
    SPD_Image image;
    SPD_DecodedInfo info;
    SPD_ValidationSummary summary;
    SPD_ValidationStatus status;

    CreateValidImage(&image);  

    if (!SPD_Decode(&image, &info))
    {
        printf("Validator setup: FAIL\n");
        return 1;
    } 

    status = SPD_Validate(&image, &info, &summary); 

    if (status == SPD_STATUS_ERROR)
    {
        printf("Valid image test: FAIL\n"); 
        return 1;
    } 
 
    CreateValidImage(&image); 

    image.data[4] ^= 0x01U; 

    if (!SPD_Decode(&image, &info))
    {
        printf("Corrupt-image decode: FAIL\n"); 
        return 1;
    }

    status = SPD_Validate(&image, &info, &summary); 

    if (status != SPD_STATUS_ERROR)
    {
        printf("CRC corruption detection: FAIL\n"); 
        return 1;
    }

    printf("Validator tests: PASS\n"); 
    return 0;
}      
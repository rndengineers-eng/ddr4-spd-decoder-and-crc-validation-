#ifndef SPD_VALIDATOR_H
#define SPD_VALIDATOR_H

#include "spd_decoder.h"

typedef enum 
{
    SPD_STATUS_PASS = 0,  
    SPD_STATUS_WARNING,
    SPD_STATUS_ERROR, 
    SPD_STATUS_NOT_CHECKED

} SPD_ValidationStatus;

typedef struct
{
    unsigned int pass_count;
    unsigned int warning_count;
    unsigned int error_count;
    unsigned int not_checked_count;

} SPD_ValidationSummary;

SPD_ValidationStatus SPD_Validate(
    const SPD_Image *image,
    const SPD_DecodedInfo *info,
    SPD_ValidationSummary *summary
); 

void SPD_PrintValidationSummary(
    const SPD_ValidationSummary *summary
); 

#endif        
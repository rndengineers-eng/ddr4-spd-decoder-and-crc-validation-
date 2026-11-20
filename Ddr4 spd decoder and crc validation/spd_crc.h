 #ifndef SPD_CRC_H
#define SPD_CRC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

uint16_t SPD_Crc16(const uint8_t *data, size_t length); 

bool SPD_VerifyBaseCrc(const uint8_t *spd, size_t size, uint16_t *calculated, uint16_t*stored);  

bool SPD_VerifyBlock1Crc(const uint8_t *spd, size_t size, uint16_t *calculated, uint16_t*stored);  

#endif       
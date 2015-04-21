/*
  Name  : CRC-8
  Poly  : 0x31    x^8 + x^5 + x^4 + 1
  Init  : 0xFF
  Revert: false
  XorOut: 0x00
  Check : 0xF7 ("123456789")
  MaxLen: 15 bytes (127 bits) - обнаружение одинарных, двойных, тройных и всех нечетных ошибок
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CRC8_H
#define __CRC8_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */  
void Crc8Init(uint8_t *crc);
void Crc8Step(uint8_t *crc, const uint8_t value);
uint8_t Crc8(uint8_t const *pcBlock, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif //__CRC8_H

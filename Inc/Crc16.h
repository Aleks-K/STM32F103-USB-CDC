/*
  Name  : CRC-16 CCITT
  Poly  : 0x1021    x^16 + x^12 + x^5 + 1
  Init  : 0xFFFF
  Revert: false
  XorOut: 0x0000
  Check : 0x29B1 ("123456789")
  MaxLen: 4095 bytes (32767 bits) - обнаружение одинарных, двойных, тройных и всех нечетных ошибок
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CRC16_H
#define __CRC16_H

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
void Crc16Init(uint16_t *crc);
void Crc16Step(uint16_t *crc, const uint8_t value);
uint16_t Crc16(uint8_t const *pcBlock, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif //__CRC16_H

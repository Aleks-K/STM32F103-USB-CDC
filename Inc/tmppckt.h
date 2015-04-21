/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TMPPCKT_H
#define __TMPPCKT_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <string.h>

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern uint8_t TmpPacket[512];
extern uint16_t TmpPacketLength;

/* Exported functions ------------------------------------------------------- */  
void TmpPacketCreate(uint16_t cmd, uint16_t ans, uint8_t* data, uint16_t dataLen);
void TmpPacketCreate2(uint8_t dev, uint8_t anscmd, uint8_t* data, uint8_t dataLen);

float Buf2Float(uint8_t* buf, uint32_t offset);

inline static void Byte2Buf(uint8_t v, uint8_t* buf) { *buf = v; }
inline static void Word2Buf(uint16_t v, uint8_t* buf) { memcpy(buf, (const uint8_t*)&v, sizeof(uint16_t)); }
inline static void DWord2Buf(uint32_t v, uint8_t* buf) { memcpy(buf, (const uint8_t*)&v, sizeof(uint32_t)); }
inline static void Float2Buf(float v, uint8_t* buf) { memcpy(buf, (const uint8_t*)&v, sizeof(float)); }

#ifdef __cplusplus
}
#endif

#endif //__TMPPCKT_H

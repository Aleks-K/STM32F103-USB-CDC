/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "flashcache_assign.h"

/* Exported types ------------------------------------------------------------*/
#pragma pack(push,1)
__packed typedef struct tPacketHeader
{
  uint16_t Marker;
  uint16_t Device;
  uint16_t Command;
  uint16_t Answer;
  uint16_t DataLen;
  uint32_t TimeMs;
  uint8_t PrioCnt;
  uint8_t HeaderCrc;
  uint16_t DataCrc;
} tPacketHeader;
#pragma pack(pop)

typedef struct CoreConfig_t
{
  uint8_t DummyParam1:1;
  uint8_t DummyParam2:1;
  uint8_t DummyParam3:1;
} CoreConfig_t;

/* Exported constants --------------------------------------------------------*/
#define FwVer 0x0000
#define HwVer 0x0000

#define UsbRxBufSize 512 // Буффер для приема по USB

#define MaxPacketDataLength2 384


#define RebootKey 0xaa55

#define Marker 0x3e23 // "#>"
#define DeviceId 0x1233
#define AnswerAck 0x5a00
#define AnswerNack 0x5aff
#define Marker0 (Marker&0xff) //0x23
#define Marker1 ((Marker>>8)&0xff) //0x3e
#define Device0 (DeviceId&0xff) // 0x33
#define Device1 ((DeviceId>>8)&0xff) // 0x12


#define M_PI 3.1415926


/* Exported macro ------------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

//==============================================================================
#if defined(__TIME__) & defined(__DATE__)
# define DIGIT(s, no) ((s)[no] - '0')
  
# define BUILD_TIME_H (10 * DIGIT(__TIME__, 0) + DIGIT(__TIME__, 1))
# define BUILD_TIME_M (10 * DIGIT(__TIME__, 3) + DIGIT(__TIME__, 4))
# define BUILD_TIME_S (10 * DIGIT(__TIME__, 6) + DIGIT(__TIME__, 7))
# define BUILD_TIME (BUILD_TIME_H * 60L * 60L + BUILD_TIME_M * 60L + BUILD_TIME_S)

# define BUILD_DATE_Y (1000 * DIGIT(__DATE__, 7) + 100 * DIGIT(__DATE__, 8) \
                   + 10 * DIGIT(__DATE__, 9) + DIGIT(__DATE__, 10))
# define BUILD_DATE_YY (10 * DIGIT(__DATE__, 9) + DIGIT(__DATE__, 10))
// Jan - 1, Feb - 2, ... Dec - 12
//
// Use the last letter as primary "key" and middle letter whenever
// two months end in the same letter.
# define BUILD_DATE_M (__DATE__[2] == 'b' ? 2 : \
                        (__DATE__[2] == 'y' ? 5 : \
                           (__DATE__[2] == 'l' ? 7 : \
                              (__DATE__[2] == 'g' ? 8 : \
                                 (__DATE__[2] == 'p' ? 9 : \
                                    (__DATE__[2] == 't' ? 10 : \
                                       (__DATE__[2] == 'v' ? 11 : \
                                          (__DATE__[2] == 'c' ? 12 : \
                                             (__DATE__[2] == 'n' ? \
                                              (__DATE__[1] == 'a' ? 1 : 6) : \
                                                (__DATE__[1] == 'a' ? 3 : 4))))))))))
# define BUILD_DATE_D ( 10 * (__DATE__[4] == ' ' ? 0 : DIGIT(__DATE__, 4)) + DIGIT(__DATE__, 5))
//# undef DIGIT
#endif
//==============================================================================

#endif // __MAIN_H

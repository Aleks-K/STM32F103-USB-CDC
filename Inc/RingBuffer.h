/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RingBuffer_H
#define __RingBuffer_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/
typedef struct
{
  uint32_t BufSize;
  volatile uint8_t* Buf;
  volatile uint32_t BufRdPtr;
  uint32_t BufWrPtr;
  volatile uint8_t RestartFl:1, BufWrPtrOvfFl:1, BufEmptyFl:1;
} RingBuffer_t;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */  
void RB_Init(RingBuffer_t* buf, uint32_t size);
void RB_InitStatic(RingBuffer_t* buf, uint8_t* buffer, uint32_t size);
void RB_Push(RingBuffer_t* buf, uint8_t value);
uint8_t RB_Pop(RingBuffer_t* buf);

#ifdef __cplusplus
}
#endif

#endif //__RingBuffer_H

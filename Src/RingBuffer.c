/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include "RingBuffer.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void RB_Init(RingBuffer_t* buf, uint32_t size);
void RB_InitStatic(RingBuffer_t* buf, uint8_t* buffer, uint32_t size);
void RB_PushRx(RingBuffer_t* buf, uint8_t value);
uint8_t RB_PopRx(RingBuffer_t* buf);

/* Private functions ---------------------------------------------------------*/

void RB_Init(RingBuffer_t* buf, uint32_t size)
{
  buf->BufSize = 0;
  buf->Buf = NULL;
  if (size != 0)
    buf->Buf = malloc(size);
  if (buf->Buf != NULL)
    buf->BufSize = size;
  buf->BufRdPtr = 0;
  buf->BufWrPtr = 0;
  buf->RestartFl = 1;
  buf->BufWrPtrOvfFl = 0;
  buf->BufEmptyFl = 1;
}

void RB_InitStatic(RingBuffer_t* buf, uint8_t* buffer, uint32_t size)
{
  buf->Buf = buffer;
  buf->BufSize = size;
  buf->BufRdPtr = 0;
  buf->BufWrPtr = 0;
  buf->RestartFl = 1;
  buf->BufWrPtrOvfFl = 0;
  buf->BufEmptyFl = 1;
}

void RB_Push(RingBuffer_t* buf, uint8_t value)
{
  uint32_t index;
  if (buf->BufSize == 0) return;
  if (buf->Buf == NULL) return;
  index = buf->BufWrPtr;
  buf->Buf[index] = value;
  ++index;
  if (index >= buf->BufSize) index = 0;
  buf->BufWrPtr = index;
  if (index == buf->BufRdPtr) buf->BufWrPtrOvfFl = 1;
}

uint8_t RB_Pop(RingBuffer_t* buf)
{
  uint32_t index;
  uint8_t value;
  if (buf->BufSize == 0) return 0;
  if (buf->Buf == NULL) return 0;
  index = buf->BufRdPtr;
  if (index == buf->BufWrPtr) return 0;
  value = buf->Buf[index];
  ++index;
  if (index >= buf->BufSize) index = 0;
  buf->BufRdPtr = index;
  if (index == buf->BufWrPtr)
    buf->BufEmptyFl = 1;
  return value;
}

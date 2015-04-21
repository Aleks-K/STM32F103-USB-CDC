/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "main.h"
#include "Crc8.h"
#include "Crc16.h"
#include "tmppckt.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t TmpPacket[512];
uint16_t TmpPacketLength = 0;

/* Private function prototypes -----------------------------------------------*/
void TmpPacketCreate(uint16_t cmd, uint16_t ans, uint8_t* data, uint16_t dataLen);
void TmpPacketCreate2(uint8_t dev, uint8_t anscmd, uint8_t* data, uint8_t dataLen);

void Float2Buf(float v, uint8_t* buf);

/* Private functions ---------------------------------------------------------*/

void TmpPacketCreate(uint16_t cmd, uint16_t ans, uint8_t* data, uint16_t dataLen)
{
  uint8_t* ptr = NULL;
  // Header
  TmpPacket[0] = Marker0; TmpPacket[1] = Marker1; // Marker "#>"
  TmpPacket[2] = Device0; TmpPacket[3] = Device1; // Dev
  ptr = (uint8_t*)&cmd;
  TmpPacket[4] = *ptr++; TmpPacket[5] = *ptr++; // Cmd
  ptr = (uint8_t*)&ans;
  TmpPacket[6] = *ptr++; TmpPacket[7] = *ptr++; // Ans
  ptr = (uint8_t*)&dataLen;
  TmpPacket[8] = *ptr++; TmpPacket[9] = *ptr++; // DataLen
  uint32_t t = HAL_GetTick();
  ptr = (uint8_t*)&t;
  TmpPacket[10] = *ptr++; TmpPacket[11] = *ptr++; TmpPacket[12] = *ptr++; TmpPacket[13] = *ptr++; // CoreTimeMs
  TmpPacket[14] = 0x00;
  TmpPacket[15] = Crc8(TmpPacket, 15); // Crc-8
  if (dataLen == 0)
  {
    TmpPacketLength = 16;
    return;
  }
  // Data
  for (int i = 0; i < dataLen; ++i)
    TmpPacket[16 + 2 + i] = *data++;
  unsigned short crc16 = Crc16(TmpPacket + 16 + 2, dataLen);
  ptr = (uint8_t*)&crc16;
  TmpPacket[16] = *ptr++; TmpPacket[16 + 1] = *ptr++; // Crc-16
  TmpPacketLength = 16 + 2 + dataLen;
}

void TmpPacketCreate2(uint8_t dev, uint8_t anscmd, uint8_t* data, uint8_t dataLen)
{
  uint8_t* ptr = NULL;
  // Header
  TmpPacket[0] = 0x7e; // Marker
  TmpPacket[1] = dataLen + 7; // Length
  TmpPacket[2] = dev; // Dev
  uint32_t t = HAL_GetTick();
  ptr = (uint8_t*)&t;
  TmpPacket[3] = *ptr++; TmpPacket[4] = *ptr++; TmpPacket[5] = *ptr++; // Tick
  TmpPacket[6] = 0; // Cnt
  TmpPacket[7] = anscmd; // Cmd answer
  // Data
  for (int i = 0; i < dataLen; ++i)
    TmpPacket[8 + i] = *data++;
  unsigned short cs = 0;
  for (int i = 0; i < 8 + dataLen; ++i)
    cs += TmpPacket[i];
  TmpPacket[dataLen + 8] = cs;
  TmpPacketLength = 8 + dataLen + 1;
}

float Buf2Float(uint8_t* buf, uint32_t offset)
{
  float res;
  memcpy((uint8_t*)&res, buf, sizeof(float));
  return res;
}

/*
  Name  : CRC-16 CCITT
  Poly  : 0x1021    x^16 + x^12 + x^5 + 1
  Init  : 0xFFFF
  Revert: false
  XorOut: 0x0000
  Check : 0x29B1 ("123456789")
  MaxLen: 4095 bytes (32767 bits) - обнаружение одинарных, двойных, тройных и всех нечетных ошибок
*/

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
const uint16_t Crc16InitValue = 0xFFFF;
const uint16_t Crc16PolyValue = 0x1021;

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void Crc16Init(uint16_t *crc);
void Crc16Step(uint16_t *crc, const uint8_t value);
uint16_t Crc16(uint8_t const *pcBlock, uint32_t len);

/* Private functions ---------------------------------------------------------*/

void Crc16Init(uint16_t *crc)
{
  *crc = Crc16InitValue;
}

void Crc16Step(uint16_t *crc, const uint8_t value)
{
  *crc ^= value << 8;
  for (uint8_t i = 0; i < 8; ++i)
    *crc = ((*crc & 0x8000) != 0) ? ((*crc << 1) ^ Crc16PolyValue) : (*crc << 1);
}

uint16_t Crc16(uint8_t const *pcBlock, uint32_t len)
{
  uint16_t crc;
  Crc16Init(&crc);
  while (len--)
    Crc16Step(&crc, *pcBlock++);
  return crc;
}

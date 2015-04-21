/*
  Name  : CRC-8
  Poly  : 0x31    x^8 + x^5 + x^4 + 1
  Init  : 0xFF
  Revert: false
  XorOut: 0x00
  Check : 0xF7 ("123456789")
  MaxLen: 15 bytes (127 bits) - обнаружение одинарных, двойных, тройных и всех нечетных ошибок
*/

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
const uint8_t Crc8InitValue = 0xFF;
const uint8_t Crc8PolyValue = 0x31;

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void Crc8Init(uint8_t *crc);
void Crc8Step(uint8_t *crc, const uint8_t value);
uint8_t Crc8(uint8_t const *pcBlock, uint32_t len);

/* Private functions ---------------------------------------------------------*/

void Crc8Init(uint8_t *crc)
{
  *crc = Crc8InitValue;
}

void Crc8Step(uint8_t *crc, const uint8_t value)
{
  *crc ^= value;
  for (uint8_t i = 0; i < 8; ++i)
    *crc = ((*crc & 0x80) != 0) ? ((*crc << 1) ^ Crc8PolyValue) : (*crc << 1);
}

uint8_t Crc8(uint8_t const *pcBlock, uint32_t len)
{
  uint8_t crc;
  Crc8Init(&crc);
  while (len--)
    Crc8Step(&crc, *pcBlock++);
  return crc;
}

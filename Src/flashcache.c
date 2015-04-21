/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "flashcache.h"
#include "Crc16.h"

/* Private typedef -----------------------------------------------------------*/
typedef union
{
  uint8_t All;
  struct
  {
    uint8_t FlashWriteError:1;
    uint8_t FlashEraseError:1;
    uint8_t FlashSaveError:1;
  };
} tFlashCacheStatus;

/* Private define ------------------------------------------------------------*/
#define FC_BackupPage      (0x0801f800)
#define FC_WorkPage        (0x0801fc00)
//#define FC_PageLen         (0x0801fc00 - 0x0801f800) // in .h file
//#define FCint_BlockCount   (1)

//#define FC_BlocksInSector (1)
#define FC_MapSize        (1)

//#define FC_BlockLen (FC_PageLen)
//#define FC_MaxBlockDataLen (FC_BlockLen)

/* Private constants ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t FC_Cache[FC_CacheSizeDW];
tFlashCacheStatus FC_Status;

uint8_t FC_Map[FC_MapSize];

#pragma location = FC_WorkPage
__no_init const uint32_t FC_CacheRO[FC_CacheSizeDW];

/* Private function prototypes -----------------------------------------------*/
void FC_Init();
void FC_Clear();
uint8_t FCint_ErasePage(uint32_t PageAddress);
void FC_SaveFlash();
uint8_t FC_GetStatus();

void FC_GenFlashMap();
uint8_t* FC_GetFlashMap();
uint32_t FC_GetFlashMapSize();

uint32_t FCint_SaveFlash(uint32_t sector);

/* Private functions ---------------------------------------------------------*/

void FC_Init()
{
  for (uint32_t i = 0; i < FC_CacheSizeDW; i++)
    FC_Cache[i] = FC_CacheRO[i];
}

void FC_Clear()
{
  for (uint32_t i = 0; i < FC_CacheSizeDW; i++)
    FC_Cache[i] = 0;
}

uint8_t FCint_ErasePage(uint32_t PageAddress)
{
  uint32_t PAGEError = 0;
  FLASH_EraseInitTypeDef EraseInitStruct;
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = PageAddress;
  EraseInitStruct.NbPages = 1;
  HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);
  if (status != HAL_OK)
    return 0xff;
  return 0;
}

void FC_SaveFlash()
{
  FC_Status.FlashSaveError = 0;
  FC_Status.FlashEraseError = 0;
  HAL_FLASH_Unlock();
  //
  if (FCint_ErasePage(FC_BackupPage) != 0)
    FC_Status.FlashSaveError = FC_Status.FlashEraseError = 1;
  if (FCint_SaveFlash(FC_BackupPage) == 0)
  {
    if (FCint_ErasePage(FC_WorkPage) != 0)
      FC_Status.FlashSaveError = FC_Status.FlashEraseError = 1;
    if (FCint_SaveFlash(FC_WorkPage) != 0)
      FC_Status.FlashSaveError = 1;
    else if (FCint_ErasePage(FC_BackupPage) != 0)
      FC_Status.FlashSaveError = FC_Status.FlashEraseError = 1;
  }
  else
    FC_Status.FlashSaveError = 1;
  //
  HAL_FLASH_Lock();
}

uint8_t FC_GetStatus() { return FC_Status.All; }

/* ---------------------------------------------------------------------------*/

void FC_GenFlashMap()
{
  // 11b 0x3 free
  // 01b 0x1 fill
  // 10b 0x2 error
  // 00b 0x0 obsolete
  FC_Map[0] = 0x1;
}

uint8_t* FC_GetFlashMap() { return FC_Map; }

uint32_t FC_GetFlashMapSize() { return FC_MapSize; }

/* ---------------------------------------------------------------------------*/

uint32_t FCint_SaveFlash(uint32_t sector)
{
  FC_Status.FlashWriteError = 0;
  for (uint32_t i = 0; i < FC_CacheSizeDW; i++)
  {
    HAL_StatusTypeDef fs = HAL_OK;
    fs = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, sector + i * 4, FC_Cache[i]);
    if (fs != HAL_OK)
      FC_Status.FlashWriteError = 1;
  }
  if (FC_Status.FlashWriteError) return 1;
  return 0;
}

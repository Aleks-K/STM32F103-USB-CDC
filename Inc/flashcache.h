/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASHCACHE_H
#define __FLASHCACHE_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "stm32f1xx_hal.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define FC_PageLen         (FLASH_PAGE_SIZE)
#define FC_CacheSizeDW     (FC_PageLen / 4) // 256 DW data = 1 KB in RAM

/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern uint32_t FC_Cache[FC_CacheSizeDW];

/* Exported functions ------------------------------------------------------- */  
void FC_Init();
void FC_Clear();
void FC_SaveFlash();
uint8_t FC_GetStatus();

void FC_GenFlashMap();
uint8_t* FC_GetFlashMap();
uint32_t FC_GetFlashMapSize();

#ifdef __cplusplus
}
#endif

#endif //__FLASHCACHE_H

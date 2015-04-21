/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASHCACHE_ASSIGN_H
#define __FLASHCACHE_ASSIGN_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "flashcache.h" // UNDONE: Rename to FlashCacheDriver.h
#include "main.h" //types.h

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define HwSerialNumber  (*(const uint32_t*)&FC_Cache[0]) // Аппаратный серийный номер
#define CoreConfig      (*(const CoreConfig_t*)&FC_Cache[1]) // Конфигурация
// --- END CACHE --- 0-255 --- LEN 256 ---


/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */  
uint8_t FC_Check();

#ifdef __cplusplus
}
#endif

#endif //__FLASHCACHE_ASSIGN_H

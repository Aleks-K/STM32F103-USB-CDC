/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __NaviPacketProc_H
#define __NaviPacketProc_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "RingBuffer.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
// Packet answer modificator
#define PKID_Answer       0x8000
// Common command packet codes
#define PKID_CmdEcho      0x0000
#define PKID_CmdGetInfo   0x0001
//#define PKID_CmdMemRead   0x0002
//#define PKID_CmdMemWrite  0x0003
#define PKID_CmdReset     0x000f
// Flash cache command packet codes
#define PKID_CmdFlGetInfo 0x0010
#define PKID_CmdFlReread  0x0011
#define PKID_CmdFlRead    0x0012
#define PKID_CmdFlWrite   0x0013
#define PKID_CmdFlSave    0x0014
#define PKID_CmdFlGetMap  0x0015
//#define PKID_CmdFlClear   0x001f

/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */  
void MasterPacketDecoder_V1(uint8_t * inBuf, uint16_t len);
void InfoTmpPacketCreate();
void UsbRxProc(RingBuffer_t *buf);

#ifdef __cplusplus
}
#endif

#endif //__NaviPacketProc_H

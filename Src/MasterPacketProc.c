/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <time.h>
#include "main.h"
#include "usbd_cdc_if.h"
#include "Crc8.h"
#include "Crc16.h"
#include "tmppckt.h"
#include "MasterPacketProc.h"
#include <math.h>
// TODO: To automatic generate gitinfo.inc put to project options (Alt+F7):
// 1. General options / Build Actions - Pre-build command line:
// $PROJ_DIR$\..\gitinfo\gitinfo.exe
// 2. General options / C/C++ Compiller - Proprocessor - Additional include directories:
// $PROJ_DIR$\..\gitinfo
#include "gitinfo.inc"
#include "flashcache.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t nRxRdPtrSaved2 = 0;
uint32_t nPcktPosCnt2 = 0, nPcktPosCntDataEnd2 = 0, nPcktPosCnt2v2 = 0;
uint8_t bFullPckt2[16+2+MaxPacketDataLength2], nPcktPosLen2 = 0;
//
uint16_t Device02, Command2, Answer2, DataHeaderLength2;
uint32_t PacketTick2;
uint8_t PrioCnt2;

/* Private function prototypes -----------------------------------------------*/
void MasterPacketDecoder_V1(uint8_t * inBuf, uint16_t len);
void InfoTmpPacketCreate();
void UsbRxProc(RingBuffer_t *buf);

/* Private functions ---------------------------------------------------------*/

void MasterPacketDecoder_V1(uint8_t * inBuf, uint16_t len)
{
  tPacketHeader *header = (tPacketHeader*)inBuf;
  //
  TmpPacketLength = 0;
  if (header->Device != DeviceId) return;
  switch (header->Command)
  {
    case PKID_CmdEcho:
      // Echo
      if (header->Answer != AnswerAck) break;
      TmpPacketCreate(PKID_Answer | header->Command, AnswerAck, NULL, 0);
    case PKID_Answer | PKID_CmdEcho:
      break;
    case PKID_CmdGetInfo:
      // Get information (short->long) // header->Answer=Serial Data=FwVer+FwDate+(LongSerial+FreeFlashCacheSectorPart)
      InfoTmpPacketCreate();
      break;
    case PKID_CmdReset:
      // Reset (short->short) header->Answer = Password
      if (header->Answer == RebootKey)
      {
        //TmpPacketCreate(PKID_Answer | header->Command, AnswerAck, NULL, 0);
        //for (uint16_t i = 0; i < TmpPacketLength; ++i)
        //  UBuf_PushTx(&Usart1Buf, TmpPacket[i]);
        //while (Usart1Buf.TxBufRdPtr != Usart1Buf.TxBufWrPtr)
        //  UBuf_CheckAndStartTx(&Usart1Buf);
        NVIC_SystemReset();
        break;
      }
      TmpPacketCreate(PKID_Answer | header->Command, AnswerNack, NULL, 0);
      break;
      // -----------------------------------------------------------------------
    case PKID_CmdFlGetInfo:
      // Flash info (short->long) header->Answer = Cache size
      TmpPacketCreate(PKID_Answer | header->Command, FC_CacheSizeDW, NULL, 0);
      break;
    case PKID_CmdFlReread:
      // Flash reread (short->short)
      FC_Init();
      TmpPacketCreate(PKID_Answer | header->Command, AnswerAck, NULL, 0);
      break;
    case PKID_CmdFlRead:
      // Flash read (short->long) header->Answer = FlashAdr
      if (header->Answer < FC_CacheSizeDW)
      {
        uint8_t buf[6];
        memcpy(buf + 0, &header->Answer, 2);
        memcpy(buf + 2, &FC_Cache[header->Answer], 4);
        TmpPacketCreate(PKID_Answer | header->Command, AnswerAck, buf, 6);
        break;
      }
      TmpPacketCreate(PKID_Answer | header->Command, AnswerNack, (uint8_t*)&header->Answer, 2);
      break;
    case PKID_CmdFlWrite:
      // Flash write (long->long) header->Answer = FlashAdr, Data = FlashData
      if (header->Answer < FC_CacheSizeDW)
      {
        memcpy(&FC_Cache[header->Answer], inBuf + 16 + 2, 4);
        uint8_t buf[6];
        memcpy(buf + 0, &header->Answer, 2);
        memcpy(buf + 2, &FC_Cache[header->Answer], 4);
        TmpPacketCreate(PKID_Answer | header->Command, AnswerAck, buf, 6);
        break;
      }
      TmpPacketCreate(PKID_Answer | header->Command, AnswerNack, (uint8_t*)&header->Answer, 2);
      break;
    case PKID_CmdFlSave:
      // Flash save (short->short)
      FC_SaveFlash();
      TmpPacketCreate(PKID_Answer | header->Command, (FC_GetStatus() == 0) ? AnswerAck : AnswerNack, NULL, 0);
      break;
    case PKID_CmdFlGetMap:
      // Flash get map (short->long)
      FC_GenFlashMap();
      TmpPacketCreate(PKID_Answer | header->Command, FC_GetFlashMapSize(), FC_GetFlashMap(), FC_GetFlashMapSize());
      break;
      // -----------------------------------------------------------------------
    default:
      // Unsupported command
      TmpPacketCreate(0xffff, AnswerNack, (uint8_t*)&header->Command, 2);
      break;
  }
  // Send answer
  if (TmpPacketLength > 0)
    CDC_Transmit_FS(TmpPacket, TmpPacketLength);
}

void InfoTmpPacketCreate()
{
  // Get information (short->long) // header->Answer=Serial Data=FwVer+FwDate+HwVer+GitInfo
  struct tm t0;
  t0.tm_year = BUILD_DATE_Y - 1900;
  t0.tm_mon = BUILD_DATE_M - 1;
  t0.tm_mday = BUILD_DATE_D;
  t0.tm_hour = BUILD_TIME_H;
  t0.tm_min = BUILD_TIME_M;
  t0.tm_sec = BUILD_TIME_S;
  time_t buildTime = mktime(&t0);
  t0.tm_year = COMMIT_DATE_Y - 1900;
  t0.tm_mon = COMMIT_DATE_M - 1;
  t0.tm_mday = COMMIT_DATE_D;
  t0.tm_hour = COMMIT_TIME_H;
  t0.tm_min = COMMIT_TIME_M;
  t0.tm_sec = COMMIT_TIME_S;
  time_t commitTime = mktime(&t0);
  uint8_t buf[33];
  Word2Buf(FwVer, buf + 0);
  DWord2Buf(buildTime, buf + 2);
  Word2Buf(HwVer, buf + 6);
  DWord2Buf(COMMIT_SHA_0, buf + 8);
  DWord2Buf(COMMIT_SHA_1, buf + 12);
  DWord2Buf(COMMIT_SHA_2, buf + 16);
  DWord2Buf(COMMIT_SHA_3, buf + 20);
  DWord2Buf(COMMIT_SHA_4, buf + 24);
  DWord2Buf(commitTime, buf + 28);
  Byte2Buf(COMMIT_IS_CLEAN, buf + 32);
  TmpPacketCreate(PKID_Answer | PKID_CmdGetInfo, (uint16_t)HwSerialNumber, buf, 33);
}

void UsbRxProc(RingBuffer_t *buf)
{
  if (buf->BufSize == 0) return;
  if (buf->Buf == NULL) return;
  //
  uint32_t index = buf->BufRdPtr;
  while (index != buf->BufWrPtr)
  {
    // Read byte from ring buffer
    uint8_t v = buf->Buf[index];
    // Find packet start marker
    if ((nPcktPosCnt2 == 0) && (v == Marker0) && (nPcktPosCnt2v2 == 0))
    {
      // Save current ring buffer position
      nRxRdPtrSaved2 = index;
      // Set packet position to 1
      nPcktPosCnt2 = 1;
      // Start filling header buffer
      bFullPckt2[0] = v;
    }
    else if (nPcktPosCnt2 > 0)
    {
      // Fill header buffer
      if (nPcktPosCnt2 < 16) bFullPckt2[nPcktPosCnt2] = v;
      // Check packet position...
      if (nPcktPosCnt2 == 1)
      {
        if (v != Marker1)
        {
          // Find new packet
          nPcktPosCnt2 = 0;
          // From next byte in ring buffer
          index = nRxRdPtrSaved2;
        }
      }
      else if (nPcktPosCnt2 == 15)
      {
        // Check header crc
        if (Crc8(bFullPckt2, 15) == v)
        {
          // Parse header buffer
          uint8_t* ptr = NULL;
          ptr = (uint8_t*)&Device02;
          *ptr++ = bFullPckt2[2]; *ptr++ = bFullPckt2[3];
          ptr = (uint8_t*)&Command2;
          *ptr++ = bFullPckt2[4]; *ptr++ = bFullPckt2[5];
          ptr = (uint8_t*)&Answer2;
          *ptr++ = bFullPckt2[6]; *ptr++ = bFullPckt2[7];
          ptr = (uint8_t*)&DataHeaderLength2;
          *ptr++ = bFullPckt2[8]; *ptr++ = bFullPckt2[9];
          ptr = (uint8_t*)&PacketTick2;
          *ptr++ = bFullPckt2[10]; *ptr++ = bFullPckt2[11]; *ptr++ = bFullPckt2[12]; *ptr++ = bFullPckt2[13];
//if (PrioCnt2 == bFullPckt2[14]) { nPcktPosCnt2 = 0; } // ################
          PrioCnt2 = bFullPckt2[14];
          // Filter packets by dev (--and cmd--)
          if (DataHeaderLength2 == 0)
          {
            // Count full packet lenght
            nPcktPosCntDataEnd2 = 16;
            // Short packet
            MasterPacketDecoder_V1(bFullPckt2, nPcktPosCntDataEnd2);
            // Find new packet
            nPcktPosCnt2 = 0;
          }
          else if (DataHeaderLength2 != 0)
          {
            // Long packet
            // Count full packet lenght
            nPcktPosCntDataEnd2 = 16 + DataHeaderLength2 + 2;
            if (nPcktPosCntDataEnd2 > MaxPacketDataLength2)
            {
              // Find new packet
              nPcktPosCnt2 = 0;
            }
            else
            {
              // No need to copy heared to full packet buffer
            }
          }
          else
          {
            // Find new packet
            nPcktPosCnt2 = 0;
          }
        }
        else
        {
          // Find new packet
          nPcktPosCnt2 = 0;
          // From next byte in ring buffer
          index = nRxRdPtrSaved2;
        }
      }
      else if ((nPcktPosCnt2 > 16) && (DataHeaderLength2 == 0))
      {
        // DEBUG if all right, never be executed! OK ########################### 
        // Find new packet
        nPcktPosCnt2 = 0;
      }
      else if ((nPcktPosCnt2 > 16) && (nPcktPosCnt2 >= nPcktPosCntDataEnd2 - 1))
      {
        bFullPckt2[nPcktPosCnt2] = v;
        // Find new packet
        nPcktPosCnt2 = 0;
        // UNDONE: Check data crc ##############################################
        MasterPacketDecoder_V1(bFullPckt2, nPcktPosCntDataEnd2);
      }
      else if (nPcktPosCnt2 >= 16)
      {
        // Fill full packet buffer
        bFullPckt2[nPcktPosCnt2] = v;
      }
      // Count next packet position
      if (nPcktPosCnt2 > 0) ++nPcktPosCnt2;
    }
    // Count next ring buffer index
    ++index;
    if (index >= buf->BufSize) index = 0;
  }
  buf->BufRdPtr = index;
  if (index == buf->BufWrPtr)
    buf->BufEmptyFl = 1;
}

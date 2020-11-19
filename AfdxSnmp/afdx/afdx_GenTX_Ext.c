/******************************************************************************

    Copyright (c) AIM GmbH 2007, 79111 Freiburg, Germany.

        All rights reserved.  No part of this software may
        be published, distributed, translated or otherwise
        reproduced by any means or for any purpose without
        the prior written consent of AIM GmbH, Freiburg.

-------------------------------------------------------------------------------

    Description:

    This file implements some examples and test routines for the
    Ayl-FDX-2/4 board.
    Test functions are for extended generic transmit modes.
    Especially Buffer Queues and Transmit Sub-Queues

-------------------------------------------------------------------------------

******************************************************************************/
#define INSTRUCTION_SYNC
#define ACYC_MARK


/******************************************************************************
 *  Includes
******************************************************************************/
#include <afdx/afdx_SampleUtils.h>
#include <afdx/afdx_GenericTX.h>

/******************************************************************************
 *  Defines
 ******************************************************************************/


/******************************************************************************
 *  local Vars (Modul)
 ******************************************************************************/


/******************************************************************************
 *  extern Vars
 ******************************************************************************/
extern AiUInt32 ul_PortConfig;


/******************************************************************************
 *  local prototypes
 ******************************************************************************/
AiInt32 l_SetupDataBufferQueues(AiUInt32 ul_Handle, TY_FDX_FW_BUF_HDL *pax_TxBufferQueueHandle);
AiInt32 l_SetupTxSubQueues(AiUInt32 ul_Handle, TY_FDX_FW_BUF_HDL *pax_TxBufferQueueHandle, AiUInt32 *paul_TxSubQueueHandle);
void v_TxExtendedControlFunctions(AiUInt32 ul_Handle, AiUInt32 *paul_SubQueueHdl);
void v_TxExtendedStartStopTest(AiUInt32 ul_Handle);


/*
 ##############################################################################
 #  Start of Code
 ##############################################################################
 */



/****************************************************************************
   Function    : l_SetTxQueue
-----------------------------------------------------------------------------
   Create      : 10.11.2005 rh,AIM
-----------------------------------------------------------------------------

   Inputs      : -

   Outputs     : -

   Description : Testfunction for Init TX Port and create TX Queue.

*****************************************************************************/
AiInt32 l_SetTxQueueExtended(AiUInt32 ul_Handle, TY_FDX_FW_BUF_HDL *pax_TxBufferQueueHandle, AiUInt32 *paul_TxSubQueueHandle, AiUInt32 ul_NumOfBufferQueues)
{
int i;

TY_FDX_PORT_INIT_IN x_PortInitIn;
TY_FDX_PORT_INIT_OUT x_PortInitOut;
TY_FDX_TX_MODE_CTRL x_TxModeControl;
TY_FDX_TX_QUEUE_SETUP x_TxQueueCreate;
TY_FDX_TX_QUEUE_INFO x_TxQueueInfo;
struct my_Frame_tag {
   TY_FDX_TX_FRAME_HEADER x_Frame;
   AiUInt8 uc_Data[1600];
} My_Frame;
AiUInt8  *Dt;   /* was 58 */
AiUInt8  uc_FrameNo = 0;
AiUInt8  uc_InstrNo = 0;
AiUInt16 us_FrameCount;
/*Vars for Data Buffer Queues */
TY_FDX_TX_BUF_QUEUE_DESC x_TxBufferQueueDesc;
TY_FDX_TX_BUF_QUEUE_CTRL x_TxBufferQueueCtrl;
AiUInt8 puc_Buffer[1800];

TY_FDX_TX_QUEUE_UPDATE ax_QueueUpdate[5];
void* apv_Buffers[5];
TY_TX_BUFFER_QUEUE_WRITE_INFO ax_WriteBufferInfo[5];
AiUInt32 ul_BytesWritten;

   printf("\n FdxCmdTxPortInit...");
   x_PortInitIn.ul_PortMap = 1;
   AIM_ASSERT( FdxCmdTxPortInit   (ul_Handle,&x_PortInitIn,&x_PortInitOut));

   printf("\n FdxCmdTxModeControl...");
   x_TxModeControl.ul_TransmitMode = FDX_TX_GENERIC;
   AIM_ASSERT( FdxCmdTxModeControl (ul_Handle,&x_TxModeControl));

   if (0!= ul_NumOfBufferQueues)
   {
      /* Setup Buffer Queues */
      AIM_ASSERT( l_SetupDataBufferQueues(ul_Handle, pax_TxBufferQueueHandle));
   }

   /* Setup Tx Sub Queues*/
   AIM_ASSERT( l_SetupTxSubQueues(ul_Handle, pax_TxBufferQueueHandle, paul_TxSubQueueHandle));

   printf("\n FdxCmdTXQueueCreate...");
   x_TxQueueCreate.ul_QueueSize = 0;
   AIM_ASSERT( FdxCmdTxQueueCreate (ul_Handle,&x_TxQueueCreate,&x_TxQueueInfo));

   memset(&My_Frame.x_Frame, 0, sizeof(TY_FDX_TX_FRAME_HEADER) );
   memset(puc_Buffer,0,sizeof(puc_Buffer));

   /* Setup Frame Frame Header in general  +++++++++++++++++++++++++++++++++++++++++++++++++++++ */
   My_Frame.x_Frame.uc_FrameType                            = FDX_TX_FRAME_STD;
   memset(&My_Frame.x_Frame.x_FrameAttrib, 0, sizeof (TY_FDX_TX_FRAME_ATTRIB) );
   My_Frame.x_Frame.x_FrameAttrib.uc_NetSelect              = FDX_TX_FRAME_BOTH;
   My_Frame.x_Frame.x_FrameAttrib.uc_ExternalStrobe         = FDX_DIS;
   My_Frame.x_Frame.x_FrameAttrib.uc_FrameStartMode         = FDX_TX_FRAME_START_PGWT;
   My_Frame.x_Frame.x_FrameAttrib.uc_PayloadBufferMode      = FDX_TX_FRAME_PBM_UDP;
   My_Frame.x_Frame.x_FrameAttrib.uc_PayloadGenerationMode  = FDX_TX_FRAME_PGM_USER;
   My_Frame.x_Frame.x_FrameAttrib.uc_PreambleCount          = FDX_TX_FRAME_PRE_DEF;
   My_Frame.x_Frame.x_FrameAttrib.ul_BufferQueueHandle      = pax_TxBufferQueueHandle[0].ul_Handle;
   My_Frame.x_Frame.x_FrameAttrib.ul_InterFrameGap          = 25;                         /* 25=1usec;*/
   My_Frame.x_Frame.x_FrameAttrib.ul_PacketGroupWaitTime    = 500;                        /* 500=0,5msec*/
   My_Frame.x_Frame.x_FrameAttrib.ul_PhysErrorInjection     = FDX_TX_FRAME_ERR_OFF;
   My_Frame.x_Frame.x_FrameAttrib.ul_Skew                   = 50;
   My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize              = 64;
   My_Frame.x_Frame.x_FrameAttrib.uw_SequenceNumberInit     = FDX_TX_FRAME_SEQ_INIT_AUTO;
   My_Frame.x_Frame.x_FrameAttrib.uw_SequenceNumberOffset   = FDX_TX_FRAME_SEQ_OFFS_AUTO;
   My_Frame.x_Frame.x_FrameAttrib.uc_TxIntEnable            = FDX_ENA+1;
   My_Frame.x_Frame.x_FrameAttrib.ul_IntIdent               = 55; 

   /* Setup Frame No. 1 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
   uc_FrameNo++;
   us_FrameCount = 64;
   My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize              = us_FrameCount;
   My_Frame.x_Frame.x_FrameAttrib.uw_SequenceNumberInit     = FDX_TX_FRAME_SEQ_OFF;
  
   for ( i = 0 ; i<1600; i++)
      My_Frame.uc_Data[i] = (unsigned char) uc_FrameNo;

   Dt = &My_Frame.uc_Data[0];
   /* MAC*/
   Dt[ 0]=0x03;Dt[ 1]=0x00;Dt[ 2]=0x00;Dt[ 3]=0x00;                            /* Destination Address */
   *((AiUInt16*)(&Dt[04])) =  BSWAP16_MAC(0);                                  /* VL */
   Dt[ 6]=0x02;Dt[ 7]=0x00;Dt[ 8]=0x00;Dt[ 9]=0x01;Dt[10]=0x21;Dt[11]=0x20;    /* Source Address */
   Dt[12]=0x08;Dt[13]=0x00;                                                    /* Type */
   /* IP */
   Dt[14]=0x45;Dt[15]=0x00;
   *((AiUInt16*)(&Dt[16])) = BSWAP16_MAC(us_FrameCount -19);                   /* Ip Total Length */
   Dt[18]=0x00;Dt[19]=0x00;Dt[20]=0x40;Dt[21]=0x00;
   Dt[22]=0x01;Dt[23]=0x11;
   Dt[24]=0x00;Dt[25]=0x00;                                                    /* Byte 24 and 25 = Ip Header Check sum set to 0 for correct calculation*/
   *((AiUInt32*)(&Dt[26])) = BSWAP32_MAC(ul_GenerateIp("10.1.33.1") );         /* Source Address */
   *((AiUInt32*)(&Dt[30])) = BSWAP32_MAC(ul_GenerateIp("224.224.0.0") );       /* Destinatin Address*/
   *((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10 /*Word count*/));
   /* UDP */
   *((AiUInt16*)(&Dt[34])) =  BSWAP16_MAC(1);                                  /* Src-UDP */
   *((AiUInt16*)(&Dt[36])) =  BSWAP16_MAC(2);                                  /* Dst-UDP */
   Dt[38]=0x00;Dt[39]=0x19;Dt[40]=0x00;Dt[41]=0x00;
   /* Data */
   sprintf((char*)&(My_Frame.uc_Data[42]),"F No. %d, %d Byte", uc_FrameNo, us_FrameCount); 

   printf("\n\n FdxCmdTxQueueWrite Frame %d ...", uc_FrameNo);
   AIM_ASSERT( FdxCmdTxQueueWrite (ul_Handle,FDX_TX_FRAME_HEADER_GENERIC,1,sizeof(My_Frame),&My_Frame));

   printf("\n");

//#ifdef INSTRUCTION_NOP
   /* Setup Instruction  No. 1 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
   uc_InstrNo++;
   My_Frame.x_Frame.uc_FrameType                      = FDX_TX_FRAME_INSTR;
   memset(&My_Frame.x_Frame.x_InstrAttrib, 0, sizeof(TY_FDX_TX_INSTR_ATTRIB));
   My_Frame.x_Frame.x_InstrAttrib.uc_Code             = FDX_TX_FRAME_INSTR_NOP;
   My_Frame.x_Frame.x_InstrAttrib.uc_Interrupt        = FDX_DIS;
   My_Frame.x_Frame.x_InstrAttrib.uc_NumOfSubQueues   = 0;
   My_Frame.x_Frame.x_InstrAttrib.uc_ActivSubQueue    = 0;
   My_Frame.x_Frame.x_InstrAttrib.aul_SubQueueHandle[0]  = 0;

   printf("\n FdxCmdTxQueueWrite Instr.  %d ...", uc_InstrNo);
   AIM_ASSERT( FdxCmdTxQueueWrite (ul_Handle,FDX_TX_FRAME_HEADER_GENERIC,1,sizeof(My_Frame),&My_Frame));
//#endif
   /* Setup Frame No. 2 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
   uc_FrameNo++;
   us_FrameCount = 128;
   My_Frame.x_Frame.uc_FrameType                            = FDX_TX_FRAME_STD;
   My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize              = us_FrameCount;
   My_Frame.x_Frame.x_FrameAttrib.uc_PayloadBufferMode      = FDX_TX_FRAME_PBM_STD;
   My_Frame.x_Frame.x_FrameAttrib.ul_BufferQueueHandle      = 0;
   My_Frame.x_Frame.x_FrameAttrib.uw_SequenceNumberInit     = FDX_TX_FRAME_SEQ_INIT_AUTO;
   My_Frame.x_Frame.x_FrameAttrib.uc_NetSelect              = FDX_TX_FRAME_DLY_A;
   My_Frame.x_Frame.x_FrameAttrib.uc_TxIntEnable            = FDX_ENA;
   My_Frame.x_Frame.x_FrameAttrib.ul_IntIdent               = 3; 

   for (  i = 0 ; i<1600; i++)
      My_Frame.uc_Data[i] = uc_FrameNo;

   Dt = &My_Frame.uc_Data[0];
   /* MAC */
   Dt[ 0]=0x03;Dt[ 1]=0x00;Dt[ 2]=0x00;Dt[ 3]=0x00;                           /* Destination Address */
   *((AiUInt16*)(&Dt[04])) =  BSWAP16_MAC(10);                                /* VL */
   Dt[ 6]=0x02;Dt[ 7]=0x00;Dt[ 8]=0x00;Dt[ 9]=0x01;Dt[10]=0x21;Dt[11]=0x20;   /* Source Address */
   Dt[12]=0x08;Dt[13]=0x00;                                                   /* Type */
   /* IP */
   Dt[14]=0x45;Dt[15]=0x00;
   *((AiUInt16*)(&Dt[16])) =  BSWAP16_MAC(us_FrameCount -19);                 /* Ip Total Length */
   Dt[18]=0x00;Dt[19]=0x00;Dt[20]=0x40;Dt[21]=0x00;
   Dt[22]=0x01;Dt[23]=0x11;
   Dt[24]=0x00;Dt[25]=0x00;                                                   /* Byte 24 and 25 = Ip Header Check sum set to 0 for correct calculation*/
   *((AiUInt32*)(&Dt[26])) = BSWAP32_MAC(ul_GenerateIp("10.1.33.1") );        /* Source Address */
   *((AiUInt32*)(&Dt[30])) = BSWAP32_MAC(ul_GenerateIp("224.224.0.10") );     /* Destinatin Address*/
   *((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
   /* UDP */
   *((AiUInt16*)(&Dt[34])) =  BSWAP16_MAC(1);                                 /* Src-UDP */
   *((AiUInt16*)(&Dt[36])) =  BSWAP16_MAC(2);                                 /* Dst-UDP */
   Dt[38]=0x00;Dt[39]=0x19;Dt[40]=0x00;Dt[41]=0x00;
   /* Data */
   sprintf((char*)&Dt[42],"F No. %d, %d Byte", uc_FrameNo, us_FrameCount); 

   printf("\n\n FdxCmdTxQueueWrite Frame %d ...", uc_FrameNo);
   AIM_ASSERT( FdxCmdTxQueueWrite (ul_Handle,FDX_TX_FRAME_HEADER_GENERIC,1,sizeof(My_Frame),&My_Frame));

   /* Setup Instruction  No. 2 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
   uc_InstrNo++;
   My_Frame.x_Frame.uc_FrameType                      = FDX_TX_FRAME_INSTR;
   memset(&My_Frame.x_Frame.x_InstrAttrib, 0, sizeof(TY_FDX_TX_INSTR_ATTRIB));
   My_Frame.x_Frame.x_InstrAttrib.uc_Code             = FDX_TX_FRAME_INSTR_CALL;
   My_Frame.x_Frame.x_InstrAttrib.uc_Interrupt        = FDX_DIS;
   My_Frame.x_Frame.x_InstrAttrib.uc_NumOfSubQueues   = 3;
   My_Frame.x_Frame.x_InstrAttrib.uc_ActivSubQueue    = 0;
   My_Frame.x_Frame.x_InstrAttrib.aul_SubQueueHandle[0]  = paul_TxSubQueueHandle[0];
   My_Frame.x_Frame.x_InstrAttrib.aul_SubQueueHandle[1]  = paul_TxSubQueueHandle[1];
   My_Frame.x_Frame.x_InstrAttrib.aul_SubQueueHandle[2]  = paul_TxSubQueueHandle[2];

   printf("\n FdxCmdTxQueueWrite Instr.  %d ...", uc_InstrNo);
   AIM_ASSERT( FdxCmdTxQueueWrite (ul_Handle,FDX_TX_FRAME_HEADER_GENERIC,1,sizeof(My_Frame),&My_Frame));

   /* Setup Frame No. 3 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
   uc_FrameNo++;
   us_FrameCount = 130;
   My_Frame.x_Frame.uc_FrameType                            = FDX_TX_FRAME_STD;
   My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize              = us_FrameCount;
   My_Frame.x_Frame.x_FrameAttrib.uc_NetSelect              = FDX_TX_FRAME_ONLY_B;
   My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize              = us_FrameCount;
   My_Frame.x_Frame.x_FrameAttrib.uc_TxIntEnable            = FDX_DIS;
   My_Frame.x_Frame.x_FrameAttrib.ul_IntIdent               = 0; 
   /* Enable this to test trigger behaviour 
   My_Frame.x_Frame.x_FrameAttrib.uc_FrameStartMode         = FDX_TX_FRAME_START_TRG_D;
   My_Frame.x_Frame.x_FrameAttrib.uc_ExternalStrobe         = FDX_ENA;
   */
   for (  i = 0 ; i<1600; i++)
      My_Frame.uc_Data[i] = uc_FrameNo;

   /* MAC */
   Dt[ 0]=0x03;Dt[ 1]=0x00;Dt[ 2]=0x00;Dt[ 3]=0x00;                           /* Destination Address */
   *((AiUInt16*)(&Dt[04])) =  BSWAP16_MAC(100);                               /* VL */
   Dt[ 6]=0x02;Dt[ 7]=0x00;Dt[ 8]=0x00;Dt[ 9]=0x01;Dt[10]=0x21;Dt[11]=0x20;   /* Source Address */
   Dt[12]=0x08;Dt[13]=0x00;                                                   /* Type */
   /* IP */
   Dt[14]=0x45;Dt[15]=0x00;
   *((AiUInt16*)(&Dt[16])) =  BSWAP16_MAC(us_FrameCount -19);                 /* Ip Total Length */
   Dt[18]=0x00;Dt[19]=0x00;Dt[20]=0x40;Dt[21]=0x00;
   Dt[22]=0x01;Dt[23]=0x11;
   Dt[24]=0x00;Dt[25]=0x00;                                                   /* Byte 24 and 25 = Ip Header Check sum set to 0 for correct calculation*/
   *((AiUInt32*)(&Dt[26])) = BSWAP32_MAC(ul_GenerateIp("10.1.33.1") );        /* Source Address */
   *((AiUInt32*)(&Dt[30])) = BSWAP32_MAC(ul_GenerateIp("224.224.0.100") );    /* Destinatin Address*/
   *((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
   /* UDP */
   *((AiUInt16*)(&Dt[34])) =  BSWAP16_MAC(1);                                 /* Src-UDP */
   *((AiUInt16*)(&Dt[36])) =  BSWAP16_MAC(2);                                 /* Dst-UDP */
   Dt[38]=0x00;Dt[39]=0x19;Dt[40]=0x00;Dt[41]=0x00;
   /* Data */
   sprintf((char*)&Dt[42],"F No. %d, %d Byte", uc_FrameNo, us_FrameCount); 

   printf("\n FdxCmdTxQueueWrite Frame %d ...", uc_FrameNo);
   AIM_ASSERT( FdxCmdTxQueueWrite (ul_Handle,FDX_TX_FRAME_HEADER_GENERIC,1,sizeof(My_Frame),&My_Frame));

   /* Setup Instruction  No. 3 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
   uc_InstrNo++;
   My_Frame.x_Frame.uc_FrameType                      = FDX_TX_FRAME_INSTR;
   memset(&My_Frame.x_Frame.x_InstrAttrib, 0, sizeof(TY_FDX_TX_INSTR_ATTRIB));
   My_Frame.x_Frame.x_InstrAttrib.uc_Code             = FDX_TX_FRAME_INSTR_CALL;
   My_Frame.x_Frame.x_InstrAttrib.uc_Interrupt        = FDX_DIS;
   My_Frame.x_Frame.x_InstrAttrib.uc_NumOfSubQueues   = 1;
   My_Frame.x_Frame.x_InstrAttrib.uc_ActivSubQueue    = 0;
   My_Frame.x_Frame.x_InstrAttrib.aul_SubQueueHandle[0]  = paul_TxSubQueueHandle[2];

   printf("\n FdxCmdTxQueueWrite Instr.  %d ...", uc_InstrNo);
   AIM_ASSERT( FdxCmdTxQueueWrite (ul_Handle,FDX_TX_FRAME_HEADER_GENERIC,1,sizeof(My_Frame),&My_Frame));
   /* Setup Frame No. 4 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
   uc_FrameNo++;
   us_FrameCount = 520;
   My_Frame.x_Frame.uc_FrameType                            = FDX_TX_FRAME_STD;
   My_Frame.x_Frame.x_FrameAttrib.uc_PayloadBufferMode      = FDX_TX_FRAME_PBM_FULL;
   My_Frame.x_Frame.x_FrameAttrib.ul_BufferQueueHandle      = pax_TxBufferQueueHandle[1].ul_Handle;
   My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize              = us_FrameCount;
   My_Frame.x_Frame.x_FrameAttrib.uw_SequenceNumberInit     = FDX_TX_FRAME_SEQ_OFF;
   My_Frame.x_Frame.x_FrameAttrib.uc_FrameStartMode         = FDX_TX_FRAME_START_PGWT;
   My_Frame.x_Frame.x_FrameAttrib.uc_NetSelect              = FDX_TX_FRAME_DLY_A;

   for ( i = 0 ; i<1600; i++)
      My_Frame.uc_Data[i] = (unsigned char) uc_FrameNo;

   /* MAC*/
   Dt[ 0]=0x03;Dt[ 1]=0x00;Dt[ 2]=0x00;Dt[ 3]=0x00;                           /* Destination Address */
   *((AiUInt16*)(&Dt[04])) =  BSWAP16_MAC(1000);                              /* VL */
   Dt[ 6]=0x02;Dt[ 7]=0x00;Dt[ 8]=0x00;Dt[ 9]=0x01;Dt[10]=0x21;Dt[11]=0x20;   /* Source Address */
   Dt[12]=0x08;Dt[13]=0x00;                                                   /* Type */
   /* IP */
   Dt[14]=0x45;Dt[15]=0x00;
   *((AiUInt16*)(&Dt[16])) = BSWAP16_MAC(us_FrameCount -19);                  /* Ip Total Length */
   Dt[18]=0x00;Dt[19]=0x00;Dt[20]=0x40;Dt[21]=0x00;
   Dt[22]=0x01;Dt[23]=0x11;
   Dt[24]=0x00;Dt[25]=0x00;                                                   /* Byte 24 and 25 = Ip Header Check sum set to 0 for correct calculation*/
   *((AiUInt32*)(&Dt[26])) = BSWAP32_MAC(ul_GenerateIp("10.1.33.1") );        /* Source Address */
   *((AiUInt32*)(&Dt[30])) = BSWAP32_MAC(ul_GenerateIp("224.224.3.232") );    /* Destinatin Address*/
   *((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
   /* UDP */
   *((AiUInt16*)(&Dt[34])) =  BSWAP16_MAC(1);                                 /* Src-UDP */
   *((AiUInt16*)(&Dt[36])) =  BSWAP16_MAC(2);                                 /* Dst-UDP */
   Dt[38]=0x00;Dt[39]=0x19;Dt[40]=0x00;Dt[41]=0x00;
   /* Data */
   /* Setup the Frame buffers */
   /* First copy the buffers to have two buffers for setup with one call*/
   memcpy(&My_Frame.uc_Data[1000], &My_Frame.uc_Data[0], us_FrameCount);

   sprintf((char*)&My_Frame.uc_Data[  42],"F No. %d, %d Byte (first Buffer)", uc_FrameNo, us_FrameCount); 
   sprintf((char*)&My_Frame.uc_Data[1042],"F No. %d, %d Byte (second Buffer)", uc_FrameNo, us_FrameCount); 

   printf("\n FdxCmdTxBufferQueueWrite write 2 buffers ...");

   ax_WriteBufferInfo[0].x_TxBufferQueueHandle = pax_TxBufferQueueHandle[1];
   ax_WriteBufferInfo[0].ul_StartIndex         = 0;
   ax_WriteBufferInfo[0].ul_StartByte          = 0;
   ax_WriteBufferInfo[0].ul_BytesToWrite       = us_FrameCount;

   ax_WriteBufferInfo[1].x_TxBufferQueueHandle = pax_TxBufferQueueHandle[1];
   ax_WriteBufferInfo[1].ul_StartIndex         = 1;
   ax_WriteBufferInfo[1].ul_StartByte          = 0;
   ax_WriteBufferInfo[1].ul_BytesToWrite       = us_FrameCount;

   apv_Buffers[0] = &My_Frame.uc_Data;
   apv_Buffers[1] = &My_Frame.uc_Data[1000];


   AIM_ASSERT( FdxCmdTxBufferQueueWrite ( ul_Handle,
                                          pax_TxBufferQueueHandle[1],
                                          0,
                                          0,
                                          ax_WriteBufferInfo[0].ul_BytesToWrite,
                                          apv_Buffers[0],
                                          &ul_BytesWritten) );

   AIM_ASSERT( FdxCmdTxBufferQueueWrite ( ul_Handle,
                                          pax_TxBufferQueueHandle[1],
                                          1,
                                          0,
                                          ax_WriteBufferInfo[1].ul_BytesToWrite,
                                          apv_Buffers[1],
                                          &ul_BytesWritten) );


   printf("\n FdxCmdTxQueueWrite Frame %d ...", uc_FrameNo);
   AIM_ASSERT( FdxCmdTxQueueWrite(ul_Handle,FDX_TX_FRAME_HEADER_GENERIC,1,sizeof(My_Frame),&My_Frame));

   x_TxBufferQueueCtrl.ul_BufferIndex = 1;
   x_TxBufferQueueCtrl.ul_BufferQueueMode = FDX_TX_BUF_QUEUE_KEEP;


   AIM_ASSERT( FdxCmdTxBufferQueueCtrl (  ul_Handle,
                                                pax_TxBufferQueueHandle[1],
                                                &x_TxBufferQueueCtrl,
                                                &x_TxBufferQueueDesc) ) ;


   /* Setup Frame No. 5 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
   uc_FrameNo++;
   us_FrameCount = 64;
   My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize              = us_FrameCount;
   My_Frame.x_Frame.x_FrameAttrib.uc_PayloadBufferMode      = FDX_TX_FRAME_PBM_UDP;
   My_Frame.x_Frame.x_FrameAttrib.ul_BufferQueueHandle      = pax_TxBufferQueueHandle[0].ul_Handle;
   My_Frame.x_Frame.x_FrameAttrib.uw_SequenceNumberInit     = FDX_TX_FRAME_SEQ_INIT_AUTO;
   My_Frame.x_Frame.x_FrameAttrib.uc_NetSelect              = FDX_TX_FRAME_DLY_A;
   for (  i = 0 ; i<1600; i++)
      My_Frame.uc_Data[i] = uc_FrameNo;

   /* MAC */
   Dt[ 0]=0x03;Dt[ 1]=0x00;Dt[ 2]=0x00;Dt[ 3]=0x00;                           /* Destination Address */
   *((AiUInt16*)(&Dt[04])) =  BSWAP16_MAC(10000);                             /* VL */
   Dt[ 6]=0x02;Dt[ 7]=0x00;Dt[ 8]=0x00;Dt[ 9]=0x01;Dt[10]=0x21;Dt[11]=0x20;   /* Source Address */
   Dt[12]=0x08;Dt[13]=0x00;                                                   /* Type */
   /* IP */
   Dt[14]=0x45;Dt[15]=0x00;
   *((AiUInt16*)(&Dt[16])) = BSWAP16_MAC(us_FrameCount -19);                  /* Ip Total Length */
   Dt[18]=0x00;Dt[19]=0x00;Dt[20]=0x40;Dt[21]=0x00;
   Dt[22]=0x01;Dt[23]=0x11;
   Dt[24]=0x00;Dt[25]=0x00;                                                   /* Byte 24 and 25 = Ip Header Check sum set to 0 for correct calculation*/
   *((AiUInt32*)(&Dt[26])) = BSWAP32_MAC(ul_GenerateIp("10.1.33.1") );        /* Source Address */
   *((AiUInt32*)(&Dt[30])) = BSWAP32_MAC(ul_GenerateIp("224.224.39.16") );    /* Destinatin Address*/
   *((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
   /* UDP */
   *((AiUInt16*)(&Dt[34])) =  BSWAP16_MAC(1);                                 /* Src-UDP */
   *((AiUInt16*)(&Dt[36])) =  BSWAP16_MAC(2);                                 /* Dst-UDP */
   Dt[38]=0x00;Dt[39]=0x19;Dt[40]=0x00;Dt[41]=0x00;
   /* Data */
   sprintf((char*)&Dt[42],"F No. %d, %d Byte", uc_FrameNo, us_FrameCount); 

   printf("\n FdxCmdTxQueueWrite Frame %d ...", uc_FrameNo);
   AIM_ASSERT( FdxCmdTxQueueWrite (ul_Handle,FDX_TX_FRAME_HEADER_GENERIC,1,sizeof(My_Frame),&My_Frame));

   /* Setup Frame No. 6 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
   uc_FrameNo++;
   us_FrameCount = 1518;
   My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize              = us_FrameCount;
   My_Frame.x_Frame.x_FrameAttrib.uc_PayloadBufferMode      = FDX_TX_FRAME_PBM_STD;
   My_Frame.x_Frame.x_FrameAttrib.ul_BufferQueueHandle      = 0;
   My_Frame.x_Frame.x_FrameAttrib.uc_NetSelect              = FDX_TX_FRAME_ONLY_B;
   for (  i = 0 ; i<1600; i++)
      My_Frame.uc_Data[i] = uc_FrameNo;

   My_Frame.x_Frame.x_FrameAttrib.uc_TxIntEnable            = FDX_ENA;
   My_Frame.x_Frame.x_FrameAttrib.ul_IntIdent               = 0x496e745f;     /* Int_*/


   /* MAC */
   Dt[ 0]=0x03;Dt[ 1]=0x00;Dt[ 2]=0x00;Dt[ 3]=0x00;                           /* Destination Address */
   *((AiUInt16*)(&Dt[04])) =  BSWAP16_MAC(65280);                             /* VL */
   Dt[ 6]=0x02;Dt[ 7]=0x00;Dt[ 8]=0x00;Dt[ 9]=0x01;Dt[10]=0x21;Dt[11]=0x20;   /* Source Address */
   Dt[12]=0x08;Dt[13]=0x00;                                                   /* Type */
   /* IP */
   Dt[14]=0x45;Dt[15]=0x00;
   *((AiUInt16*)(&Dt[16])) = BSWAP16_MAC(us_FrameCount -19);                  /* Ip Total Length */
   Dt[18]=0x00;Dt[19]=0x00;Dt[20]=0x40;Dt[21]=0x00;
   Dt[22]=0x01;Dt[23]=0x11;
   Dt[24]=0x00;Dt[25]=0x00;                                                   /* Byte 24 and 25 = Ip Header Check sum set to 0 for correct calculation*/
   *((AiUInt32*)(&Dt[26])) = BSWAP32_MAC(ul_GenerateIp("10.1.33.1") );        /* Source Address */
   *((AiUInt32*)(&Dt[30])) = BSWAP32_MAC(ul_GenerateIp("224.224.255.0") );    /* Destinatin Address*/
   *((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
   /* UDP */
   *((AiUInt16*)(&Dt[34])) =  BSWAP16_MAC(1);                                 /* Src-UDP */
   *((AiUInt16*)(&Dt[36])) =  BSWAP16_MAC(2);                                 /* Dst-UDP */
   Dt[38]=0x00;Dt[39]=0x19;Dt[40]=0x00;Dt[41]=0x00;
   /* Data */
   sprintf((char*)&Dt[42],"F No. %d, %d Byte", uc_FrameNo, us_FrameCount); 

   printf("\n FdxCmdTxQueueWrite Frame %d ...", uc_FrameNo);
   AIM_ASSERT( FdxCmdTxQueueWrite (ul_Handle,FDX_TX_FRAME_HEADER_GENERIC,1,sizeof(My_Frame),&My_Frame));

   /* Setup Instruction  No. 4 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
   uc_InstrNo++;
   My_Frame.x_Frame.uc_FrameType                      = FDX_TX_FRAME_INSTR;
   memset(&My_Frame.x_Frame.x_InstrAttrib, 0, sizeof(TY_FDX_TX_INSTR_ATTRIB));
   My_Frame.x_Frame.x_InstrAttrib.uc_Code             = FDX_TX_FRAME_INSTR_SYNC;
   My_Frame.x_Frame.x_InstrAttrib.uc_Interrupt        = FDX_DIS;
   My_Frame.x_Frame.x_InstrAttrib.uc_NumOfSubQueues   = 0;
   My_Frame.x_Frame.x_InstrAttrib.uc_ActivSubQueue    = 0;
   My_Frame.x_Frame.x_InstrAttrib.aul_SubQueueHandle[0]  = 0;

   printf("\n FdxCmdTxQueueWrite Instr.  %d ...",    uc_InstrNo);
   AIM_ASSERT( FdxCmdTxQueueWrite (ul_Handle,FDX_TX_FRAME_HEADER_GENERIC,1,sizeof(My_Frame),&My_Frame));
   /* Setup Instruction  No. 5 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
   uc_InstrNo++;
   My_Frame.x_Frame.uc_FrameType                      = FDX_TX_FRAME_INSTR;
   memset(&My_Frame.x_Frame.x_InstrAttrib, 0, sizeof(TY_FDX_TX_INSTR_ATTRIB));
   My_Frame.x_Frame.x_InstrAttrib.uc_Code             = FDX_TX_FRAME_INSTR_ACYC_MARK;
   My_Frame.x_Frame.x_InstrAttrib.uc_Interrupt        = FDX_DIS;
   My_Frame.x_Frame.x_InstrAttrib.uc_NumOfSubQueues   = 0;
   My_Frame.x_Frame.x_InstrAttrib.uc_ActivSubQueue    = 0;
   My_Frame.x_Frame.x_InstrAttrib.aul_SubQueueHandle[0]  = 0;

   printf("\n FdxCmdTxQueueWrite Instr.  %d ...",    uc_InstrNo);
   AIM_ASSERT( FdxCmdTxQueueWrite (ul_Handle,FDX_TX_FRAME_HEADER_GENERIC,1,sizeof(My_Frame),&My_Frame));

   /* Update Frames  No. 2 and 6  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
   ax_QueueUpdate[0].ul_Index = 2;
   ax_QueueUpdate[0].ul_Offset = 64;
   ax_QueueUpdate[0].ul_Length = 30;
   ax_QueueUpdate[0].ul_SubQueueHandle = 0;

   ax_QueueUpdate[1].ul_Index = 6;
   ax_QueueUpdate[1].ul_Offset = 80;
   ax_QueueUpdate[1].ul_Length = 50;
   ax_QueueUpdate[1].ul_SubQueueHandle = 0;

   sprintf((char*)puc_Buffer, "Updated!  Updated!  Updated!"); 
   sprintf((char*)&puc_Buffer[100], "Updated Frame6!  Updated Frame6!  Updated Frame6! "); 

   apv_Buffers[0] = puc_Buffer;
   apv_Buffers[1] = &puc_Buffer[100];

     AIM_ASSERT( FdxCmdTxQueueUpdate ( ul_Handle, 
                                       &ax_QueueUpdate[0],
                                       apv_Buffers[0] ))
     AIM_ASSERT( FdxCmdTxQueueUpdate ( ul_Handle, 
                                       &ax_QueueUpdate[1],
                                       apv_Buffers[1] ))


   printf("\n\tFrame No: %d after Queue \n",ax_QueueUpdate[0].ul_Index);
   printf(  "\tFrame No: %d after Queue \n",ax_QueueUpdate[1].ul_Index);
   return(FDX_OK);
}





/***********************************
 *
 * Setup some (three) Data Buffer Queues
 *
 *
 *
 */
AiInt32 l_SetupDataBufferQueues(AiUInt32 ul_Handle, TY_FDX_FW_BUF_HDL *pax_TxBufferQueueHandle)
{
AiInt32 r_RetVal;

/*Vars for Data Buffer Queues */
TY_FDX_TX_BUF_QUEUE_DESC x_TxBufferQueueDesc;
TY_FDX_TX_BUF_QUEUE_INFO x_TxBufferQueueInfo;
AiUInt8 *px_QueueData;
AiUInt32 ul_BytesWritten;
int ii;

   /* Setup Data Buffer Queues */
   /* first queue */
   x_TxBufferQueueDesc.ul_BufferIndex        = 0;
   x_TxBufferQueueDesc.ul_BuffersInQueue     = 6;
   x_TxBufferQueueDesc.ul_BufferSize         = 64;     
   x_TxBufferQueueDesc.ul_MaxTransfers       = 3; 
//   x_TxBufferQueueDesc.ul_BufferPayloadMode  = FDX_TX_FRAME_PBM_UDP;
   x_TxBufferQueueDesc.ul_BufferPayloadMode  = FDX_TX_FRAME_PBM_STD;
   x_TxBufferQueueDesc.ul_BufferQueueMode    = FDX_TX_BUF_QUEUE_CYC;
   
   if (FDX_OK != (r_RetVal = FdxCmdTxBufferQueueAlloc( ul_Handle,
                                                &x_TxBufferQueueDesc,
                                                &pax_TxBufferQueueHandle[0],
                                                &x_TxBufferQueueInfo) ) )
      printf("\n FdxCmdTxBufferQueueAlloc failed!\n");
   else
      printf("\n FdxCmdTxBufferQueueAlloc size:%d bytes, %d Buffers in queue. Buffer Start: %x\n", x_TxBufferQueueInfo.ul_BufferSize, x_TxBufferQueueInfo.ul_BuffersInQueue,x_TxBufferQueueInfo.pv_BufferQueueStart);
   
   /*Preset Buffers for UDP Payload*/
   px_QueueData = (AiUInt8*)malloc(x_TxBufferQueueInfo.ul_BufferSize);
   if (px_QueueData != NULL)
   {
      for (ii=0; ii < (int)x_TxBufferQueueInfo.ul_BuffersInQueue; ii++)
      {
         memset(px_QueueData, 0x40+ii, x_TxBufferQueueInfo.ul_BufferSize);
         px_QueueData[00]= 0;px_QueueData[01]= 0; /*Set UDP checksum correct */
         if (FDX_OK != (r_RetVal = FdxCmdTxBufferQueueWrite ( ul_Handle,
                                                pax_TxBufferQueueHandle[0],
                                                ii,
                                                0,
                                                x_TxBufferQueueInfo.ul_BufferSize,
                                                px_QueueData,
                                                &ul_BytesWritten) ) )
            printf("FdxCmdTxBufferQueueWrite failed!\n");
         else
            printf("FdxCmdTxBufferQueueWrite %d bytes written\n", ul_BytesWritten);
      }
      free(px_QueueData);
   }


   /* second queue */
   x_TxBufferQueueDesc.ul_BuffersInQueue     = 2;
   x_TxBufferQueueDesc.ul_BufferSize         = 520;     
   x_TxBufferQueueDesc.ul_MaxTransfers       = 1; 
   x_TxBufferQueueDesc.ul_BufferPayloadMode  = FDX_TX_FRAME_PBM_FULL;
   x_TxBufferQueueDesc.ul_BufferQueueMode    = FDX_TX_BUF_QUEUE_HOST;

   if (FDX_OK != (r_RetVal = FdxCmdTxBufferQueueAlloc( ul_Handle,
                                                &x_TxBufferQueueDesc,
                                                &pax_TxBufferQueueHandle[1],
                                                &x_TxBufferQueueInfo) ) )
      printf("FdxCmdTxBufferQueueAlloc failed!\n");
   else
      printf("FdxCmdTxBufferQueueAlloc size:%d bytes, %d Buffers in queue. Buffer Start: %x\n", x_TxBufferQueueInfo.ul_BufferSize, x_TxBufferQueueInfo.ul_BuffersInQueue,x_TxBufferQueueInfo.pv_BufferQueueStart);


   /* third queue */
   x_TxBufferQueueDesc.ul_BuffersInQueue     = 200;
   x_TxBufferQueueDesc.ul_BufferSize         = 130;     
   x_TxBufferQueueDesc.ul_BufferQueueMode    = FDX_TX_BUF_QUEUE_SNG;
   
   if (FDX_OK != (r_RetVal = FdxCmdTxBufferQueueAlloc( ul_Handle,
                                                &x_TxBufferQueueDesc,
                                                &pax_TxBufferQueueHandle[2],
                                                &x_TxBufferQueueInfo) ) )
      printf("FdxCmdTxBufferQueueAlloc failed!\n");
   else
      printf("FdxCmdTxBufferQueueAlloc size:%d bytes, %d Buffers in queue. Buffer Start: %x\n", x_TxBufferQueueInfo.ul_BufferSize, x_TxBufferQueueInfo.ul_BuffersInQueue,x_TxBufferQueueInfo.pv_BufferQueueStart);


   return r_RetVal;
}


/***********************************
 *
 * Setup some (three) Transmitter Sub Queues Queues
 *
 *
 *
 */
AiInt32 l_SetupTxSubQueues(AiUInt32 ul_Handle, TY_FDX_FW_BUF_HDL *pax_TxBufferQueueHandle, AiUInt32 *paul_TxSubQueueHandle)
{
AiInt32 r_RetVal;
TY_FDX_TX_SUB_QUEUE_CREATE_IN x_TxSubQueueCreateIn;
TY_FDX_TX_SUB_QUEUE_CREATE_OUT x_TxSubQueueCreateOut;
AiUInt32 ul_Entries;
struct my_Frame_tag {
   TY_FDX_TX_FRAME_HEADER x_Frame;
   AiUInt8 uc_Data[1600];
} My_Frame;
AiUInt8 *Dt;
AiUInt8 uc_FrameNo;
AiUInt8 uc_InstrNo;
AiUInt16 us_FrameCount;
AiUInt32 ul_SubQueueHandleIndex = 0;
int i;

   /* Create Sub Queue 1 */
   ul_Entries = 5;
   printf(" Create Tx SubQueue with %d Entries\n\r", ul_Entries);
   x_TxSubQueueCreateIn.ul_QueueSize = 2*64 * ul_Entries;
   if (FDX_OK != (r_RetVal =  FdxCmdTxSubQueueCreate( ul_Handle, &x_TxSubQueueCreateIn, &x_TxSubQueueCreateOut ) ) )
      printf("FdxCmdTxSubQueueCreate failed!\n");
   else
   {
      printf("FdxCmdTxSubQueueCreate done!\n");
      paul_TxSubQueueHandle[ul_SubQueueHandleIndex] = x_TxSubQueueCreateOut.ul_SubQueueHandle;
   }

   /* Create Sub Queue 2 */
   ul_SubQueueHandleIndex++;
   ul_Entries = 7;
   printf(" Create Tx SubQueue with %d Entries\n\r", ul_Entries);
   x_TxSubQueueCreateIn.ul_QueueSize = 2*64 * ul_Entries;
   if (FDX_OK != (r_RetVal =  FdxCmdTxSubQueueCreate( ul_Handle, &x_TxSubQueueCreateIn, &x_TxSubQueueCreateOut ) ) )
      printf("FdxCmdTxSubQueueCreate failed!\n");
   else
   {
      printf("FdxCmdTxSubQueueCreate done!\n");
      paul_TxSubQueueHandle[ul_SubQueueHandleIndex] = x_TxSubQueueCreateOut.ul_SubQueueHandle;
   }

   /* Create Sub Queue 3 */
   ul_SubQueueHandleIndex++;
   ul_Entries = 2;
   printf(" Create Tx SubQueue with %d Entries\n\r", ul_Entries);
   x_TxSubQueueCreateIn.ul_QueueSize = 2*64 * ul_Entries;
   if (FDX_OK != (r_RetVal =  FdxCmdTxSubQueueCreate( ul_Handle, &x_TxSubQueueCreateIn, &x_TxSubQueueCreateOut ) ) )
      printf("FdxCmdTxSubQueueCreate failed!\n");
   else
   {
      printf("FdxCmdTxSubQueueCreate done!\n");
      paul_TxSubQueueHandle[ul_SubQueueHandleIndex] = x_TxSubQueueCreateOut.ul_SubQueueHandle;
   }

   memset(&My_Frame.x_Frame, 0, sizeof(TY_FDX_TX_FRAME_HEADER) );

   /* Fill SubQueue 1 Frame 1 #################################################################### */
   ul_SubQueueHandleIndex = 0;
   uc_FrameNo = 60;
   us_FrameCount = 64;

   for ( i = 0 ; i<1600; i++)
      My_Frame.uc_Data[i] = (unsigned char) uc_FrameNo;

   My_Frame.x_Frame.uc_FrameType                            = FDX_TX_FRAME_STD;
   My_Frame.x_Frame.x_FrameAttrib.uc_NetSelect              = FDX_TX_FRAME_BOTH;
   My_Frame.x_Frame.x_FrameAttrib.uc_ExternalStrobe         = FDX_DIS;
   My_Frame.x_Frame.x_FrameAttrib.uc_FrameStartMode         = FDX_TX_FRAME_START_PGWT;
//   My_Frame.x_Frame.x_FrameAttrib.uc_PayloadBufferMode      = FDX_TX_FRAME_PBM_UDP;
   My_Frame.x_Frame.x_FrameAttrib.uc_PayloadBufferMode      = FDX_TX_FRAME_PBM_STD;

   My_Frame.x_Frame.x_FrameAttrib.uc_PayloadGenerationMode  = FDX_TX_FRAME_PGM_USER;
   My_Frame.x_Frame.x_FrameAttrib.uc_PreambleCount          = FDX_TX_FRAME_PRE_DEF;
   My_Frame.x_Frame.x_FrameAttrib.ul_BufferQueueHandle      = pax_TxBufferQueueHandle[0].ul_Handle;
   My_Frame.x_Frame.x_FrameAttrib.ul_InterFrameGap          = 25;                         /* 25=1usec;*/
   My_Frame.x_Frame.x_FrameAttrib.ul_PacketGroupWaitTime    = 500;                        /* 500=0,5msec*/
   My_Frame.x_Frame.x_FrameAttrib.ul_PhysErrorInjection     = FDX_TX_FRAME_ERR_OFF;
   My_Frame.x_Frame.x_FrameAttrib.ul_Skew                   = 50;
   My_Frame.x_Frame.x_FrameAttrib.uw_SequenceNumberInit     = FDX_TX_FRAME_SEQ_INIT_AUTO;
   My_Frame.x_Frame.x_FrameAttrib.uw_SequenceNumberOffset   = FDX_TX_FRAME_SEQ_OFFS_AUTO;
   My_Frame.x_Frame.x_FrameAttrib.uc_TxIntEnable            = FDX_DIS;
   My_Frame.x_Frame.x_FrameAttrib.ul_IntIdent               = 1;

   My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize              = us_FrameCount;
   
   Dt = &My_Frame.uc_Data[0];

   /* MAC */
   Dt[ 0]=0x03;Dt[ 1]=0x00;Dt[ 2]=0x00;Dt[ 3]=0x00;                           /* Destination Address */
   *((AiUInt16*)(&Dt[04])) =  BSWAP16_MAC(300);                               /* VL */
   Dt[ 6]=0x02;Dt[ 7]=0x00;Dt[ 8]=0x00;Dt[ 9]=0x01;Dt[10]=0x21;Dt[11]=0x20;   /* Source Address */
   Dt[12]=0x08;Dt[13]=0x00;                                                   /* Type */
   /* IP */
   Dt[14]=0x45;Dt[15]=0x00;
   *((AiUInt16*)(&Dt[16])) = BSWAP16_MAC(us_FrameCount -19);                  /* Ip Total Length */
   Dt[18]=0x00;Dt[19]=0x00;Dt[20]=0x40;Dt[21]=0x00;
   Dt[22]=0x01;Dt[23]=0x11;
   Dt[24]=0x00;Dt[25]=0x00;                                                   /* Byte 24 and 25 = Ip Header Check sum set to 0 for correct calculation*/
   *((AiUInt32*)(&Dt[26])) = BSWAP32_MAC(ul_GenerateIp("10.1.33.1") );        /* Source Address */
   *((AiUInt32*)(&Dt[30])) = BSWAP32_MAC(ul_GenerateIp("224.224.1.44") );    /* Destinatin Address*/
   *((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
   /* UDP */
   *((AiUInt16*)(&Dt[34])) =  BSWAP16_MAC(1001);                              /* Src-UDP */
   *((AiUInt16*)(&Dt[36])) =  BSWAP16_MAC(1002);                              /* Dst-UDP */
   Dt[38]=0x00;Dt[39]=0x19;Dt[40]=0x00;Dt[41]=0x00;
   /* Data */
   sprintf((char*)&Dt[42],"F No. %d, %d Byte", uc_FrameNo, us_FrameCount); 

   printf("\n FdxCmdTxSubQueueWrite Frame %d ...", uc_FrameNo);
   if (FDX_OK != (r_RetVal =  FdxCmdTxSubQueueWrite (    ul_Handle,
                                                      paul_TxSubQueueHandle[ul_SubQueueHandleIndex],
                                                      1, 
                                                      sizeof(My_Frame), 
                                                      &My_Frame) ) )
      printf("FdxCmdTxSubQueueWrite failed!\n");
   else
      printf("FdxCmdTxSubQueueWrite done!\n");


   /* Fill SubQueue 1 Frame 2 #################################################################### */
   uc_FrameNo = 61;
   us_FrameCount = 128;

   for ( i = 0 ; i<1600; i++)
      My_Frame.uc_Data[i] = (unsigned char) uc_FrameNo;

   My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize              = us_FrameCount;
   My_Frame.x_Frame.x_FrameAttrib.uc_PayloadBufferMode      = FDX_TX_FRAME_PBM_STD;
   My_Frame.x_Frame.x_FrameAttrib.ul_BufferQueueHandle      = 0;
   
   Dt = &My_Frame.uc_Data[0];

   /* MAC */
   Dt[ 0]=0x03;Dt[ 1]=0x00;Dt[ 2]=0x00;Dt[ 3]=0x00;                           /* Destination Address */
   *((AiUInt16*)(&Dt[04])) =  BSWAP16_MAC(310);                               /* VL */
   Dt[ 6]=0x02;Dt[ 7]=0x00;Dt[ 8]=0x00;Dt[ 9]=0x01;Dt[10]=0x21;Dt[11]=0x20;   /* Source Address */
   Dt[12]=0x08;Dt[13]=0x00;                                                   /* Type */
   /* IP */
   Dt[14]=0x45;Dt[15]=0x00;
   *((AiUInt16*)(&Dt[16])) = BSWAP16_MAC(us_FrameCount -19);                  /* Ip Total Length */
   Dt[18]=0x00;Dt[19]=0x00;Dt[20]=0x40;Dt[21]=0x00;
   Dt[22]=0x01;Dt[23]=0x11;
   Dt[24]=0x00;Dt[25]=0x00;                                                   /* Byte 24 and 25 = Ip Header Check sum set to 0 for correct calculation*/
   *((AiUInt32*)(&Dt[26])) = BSWAP32_MAC(ul_GenerateIp("10.1.33.1") );        /* Source Address */
   *((AiUInt32*)(&Dt[30])) = BSWAP32_MAC(ul_GenerateIp("224.224.1.44") );    /* Destinatin Address*/
   *((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
   /* UDP */
   *((AiUInt16*)(&Dt[34])) =  BSWAP16_MAC(1003);                              /* Src-UDP */
   *((AiUInt16*)(&Dt[36])) =  BSWAP16_MAC(1004);                              /* Dst-UDP */
   Dt[38]=0x00;Dt[39]=0x19;Dt[40]=0x00;Dt[41]=0x00;
   /* Data */
   sprintf((char*)&Dt[42],"F No. %d, %d Byte", uc_FrameNo, us_FrameCount); 

   printf("\n FdxCmdTxSubQueueWrite Frame %d ...", uc_FrameNo);

   if (FDX_OK != (r_RetVal =  FdxCmdTxSubQueueWrite (    ul_Handle,
                                                      paul_TxSubQueueHandle[ul_SubQueueHandleIndex],
                                                      1, 
                                                      sizeof(My_Frame), 
                                                      &My_Frame) ) )
      printf("FdxCmdTxSubQueueWrite failed!\n");
   else
      printf("FdxCmdTxSubQueueWrite done!\n");


   /* Fill SubQueue 1 Frame 3 #################################################################### */
   uc_FrameNo = 62;
   us_FrameCount = 257;

   for ( i = 0 ; i<1600; i++)
      My_Frame.uc_Data[i] = (unsigned char) uc_FrameNo;

   My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize              = us_FrameCount;
   My_Frame.x_Frame.x_FrameAttrib.uc_PayloadBufferMode      = FDX_TX_FRAME_PBM_STD;
   My_Frame.x_Frame.x_FrameAttrib.ul_BufferQueueHandle      = 0;
   
   Dt = &My_Frame.uc_Data[0];

   /* MAC */
   Dt[ 0]=0x03;Dt[ 1]=0x00;Dt[ 2]=0x00;Dt[ 3]=0x00;                           /* Destination Address */
   *((AiUInt16*)(&Dt[04])) =  BSWAP16_MAC(320);                               /* VL */
   Dt[ 6]=0x02;Dt[ 7]=0x00;Dt[ 8]=0x00;Dt[ 9]=0x01;Dt[10]=0x21;Dt[11]=0x20;   /* Source Address */
   Dt[12]=0x08;Dt[13]=0x00;                                                   /* Type */
   /* IP */
   Dt[14]=0x45;Dt[15]=0x00;
   *((AiUInt16*)(&Dt[16])) = BSWAP16_MAC(us_FrameCount -19);                  /* Ip Total Length */
   Dt[18]=0x00;Dt[19]=0x00;Dt[20]=0x40;Dt[21]=0x00;
   Dt[22]=0x01;Dt[23]=0x11;
   Dt[24]=0x00;Dt[25]=0x00;                                                   /* Byte 24 and 25 = Ip Header Check sum set to 0 for correct calculation*/
   *((AiUInt32*)(&Dt[26])) = BSWAP32_MAC(ul_GenerateIp("10.1.33.1") );        /* Source Address */
   *((AiUInt32*)(&Dt[30])) = BSWAP32_MAC(ul_GenerateIp("224.224.1.64") );    /* Destinatin Address*/
   *((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
   /* UDP */
   *((AiUInt16*)(&Dt[34])) =  BSWAP16_MAC(1005);                              /* Src-UDP */
   *((AiUInt16*)(&Dt[36])) =  BSWAP16_MAC(1006);                              /* Dst-UDP */
   Dt[38]=0x00;Dt[39]=0x19;Dt[40]=0x00;Dt[41]=0x00;
   /* Data */
   sprintf((char*)&Dt[42],"F No. %d, %d Byte", uc_FrameNo, us_FrameCount); 

   printf("\n FdxCmdTxSubQueueWrite Frame %d ...", uc_FrameNo);
   if (FDX_OK != (r_RetVal =  FdxCmdTxSubQueueWrite (    ul_Handle,
                                                      paul_TxSubQueueHandle[ul_SubQueueHandleIndex],
                                                      1, 
                                                      sizeof(My_Frame), 
                                                      &My_Frame) ) )
      printf("FdxCmdTxSubQueueWrite failed!\n");
   else
      printf("FdxCmdTxSubQueueWrite done!\n");

   /* Fill SubQueue 1 Frame 4 #################################################################### */
   uc_FrameNo = 63;
   us_FrameCount = 499;

   for ( i = 0 ; i<1600; i++)
      My_Frame.uc_Data[i] = (unsigned char) uc_FrameNo;

   My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize              = us_FrameCount;
   My_Frame.x_Frame.x_FrameAttrib.uc_PayloadBufferMode      = FDX_TX_FRAME_PBM_STD;
   My_Frame.x_Frame.x_FrameAttrib.ul_BufferQueueHandle      = 0;
   
   Dt = &My_Frame.uc_Data[0];

   /* MAC */
   Dt[ 0]=0x03;Dt[ 1]=0x00;Dt[ 2]=0x00;Dt[ 3]=0x00;                           /* Destination Address */
   *((AiUInt16*)(&Dt[04])) =  BSWAP16_MAC(330);                               /* VL */
   Dt[ 6]=0x02;Dt[ 7]=0x00;Dt[ 8]=0x00;Dt[ 9]=0x01;Dt[10]=0x21;Dt[11]=0x20;   /* Source Address */
   Dt[12]=0x08;Dt[13]=0x00;                                                   /* Type */
   /* IP */
   Dt[14]=0x45;Dt[15]=0x00;
   *((AiUInt16*)(&Dt[16])) = BSWAP16_MAC(us_FrameCount -19);                  /* Ip Total Length */
   Dt[18]=0x00;Dt[19]=0x00;Dt[20]=0x40;Dt[21]=0x00;
   Dt[22]=0x01;Dt[23]=0x11;
   Dt[24]=0x00;Dt[25]=0x00;                                                   /* Byte 24 and 25 = Ip Header Check sum set to 0 for correct calculation*/
   *((AiUInt32*)(&Dt[26])) = BSWAP32_MAC(ul_GenerateIp("10.1.33.1") );        /* Source Address */
   *((AiUInt32*)(&Dt[30])) = BSWAP32_MAC(ul_GenerateIp("224.224.1.74") );    /* Destinatin Address*/
   *((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
   /* UDP */
   *((AiUInt16*)(&Dt[34])) =  BSWAP16_MAC(1007);                              /* Src-UDP */
   *((AiUInt16*)(&Dt[36])) =  BSWAP16_MAC(1008);                              /* Dst-UDP */
   Dt[38]=0x00;Dt[39]=0x19;Dt[40]=0x00;Dt[41]=0x00;
   /* Data */
   sprintf((char*)&Dt[42],"F No. %d, %d Byte", uc_FrameNo, us_FrameCount); 


   printf("\n FdxCmdTxSubQueueWrite Frame %d ...", uc_FrameNo);
   if (FDX_OK != (r_RetVal =  FdxCmdTxSubQueueWrite  (    ul_Handle,
                                                      paul_TxSubQueueHandle[ul_SubQueueHandleIndex],
                                                      1, 
                                                      sizeof(My_Frame), 
                                                      &My_Frame) ) )
      printf("FdxCmdTxSubQueueWrite failed!\n");
   else
      printf("FdxCmdTxSubQueueWrite done!\n");

   /* Setup Instruction  No. 3 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
#ifdef INSTRUCTION_SYNC
   uc_InstrNo = 11;
   My_Frame.x_Frame.uc_FrameType                      = FDX_TX_FRAME_INSTR;
   memset(&My_Frame.x_Frame.x_InstrAttrib, 0, sizeof(TY_FDX_TX_INSTR_ATTRIB));
   My_Frame.x_Frame.x_InstrAttrib.uc_Code             = FDX_TX_FRAME_INSTR_SYNC;
   My_Frame.x_Frame.x_InstrAttrib.uc_Interrupt        = FDX_DIS;
   My_Frame.x_Frame.x_InstrAttrib.uc_NumOfSubQueues   = 0;
   My_Frame.x_Frame.x_InstrAttrib.uc_ActivSubQueue    = 0;
   My_Frame.x_Frame.x_InstrAttrib.aul_SubQueueHandle[0]  = 0;

   printf("\n FdxCmdTxQueueWrite Instr.  %d ...",    uc_InstrNo);
   if (FDX_OK != (r_RetVal = FdxCmdTxSubQueueWrite (    ul_Handle, 
                                                      paul_TxSubQueueHandle[ul_SubQueueHandleIndex],
                                                      1,
                                                      sizeof(My_Frame),
                                                      &My_Frame)))
       
      printf("\r\nFdxCmdTxQueueWrite() failed.");
#endif
   /* SubQueue 1 filled */      


   /* Fill SubQueue 2 Frame 1 #################################################################### */
   ul_SubQueueHandleIndex++;
   uc_FrameNo = 80;
   us_FrameCount = 64;

   for ( i = 0 ; i<1600; i++)
      My_Frame.uc_Data[i] = (unsigned char) uc_FrameNo;

   My_Frame.x_Frame.uc_FrameType                            = FDX_TX_FRAME_STD;
   My_Frame.x_Frame.x_FrameAttrib.uc_NetSelect              = FDX_TX_FRAME_BOTH;
   My_Frame.x_Frame.x_FrameAttrib.uc_ExternalStrobe         = FDX_DIS;
   My_Frame.x_Frame.x_FrameAttrib.uc_FrameStartMode         = FDX_TX_FRAME_START_PGWT;
   My_Frame.x_Frame.x_FrameAttrib.uc_PayloadBufferMode      = FDX_TX_FRAME_PBM_STD;

   My_Frame.x_Frame.x_FrameAttrib.uc_PayloadGenerationMode  = FDX_TX_FRAME_PGM_USER;
   My_Frame.x_Frame.x_FrameAttrib.uc_PreambleCount          = FDX_TX_FRAME_PRE_DEF;
   My_Frame.x_Frame.x_FrameAttrib.ul_BufferQueueHandle      = 0;
   My_Frame.x_Frame.x_FrameAttrib.ul_InterFrameGap          = 25;                         /* 25=1usec;*/
   My_Frame.x_Frame.x_FrameAttrib.ul_PacketGroupWaitTime    = 500;                        /* 500=0,5msec*/
   My_Frame.x_Frame.x_FrameAttrib.ul_PhysErrorInjection     = FDX_TX_FRAME_ERR_OFF;
   My_Frame.x_Frame.x_FrameAttrib.ul_Skew                   = 50;
   My_Frame.x_Frame.x_FrameAttrib.uw_SequenceNumberInit     = FDX_TX_FRAME_SEQ_INIT_AUTO;
   My_Frame.x_Frame.x_FrameAttrib.uw_SequenceNumberOffset   = FDX_TX_FRAME_SEQ_OFFS_AUTO;

   My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize              = us_FrameCount;
   
   Dt = &My_Frame.uc_Data[0];

   /* MAC */
   Dt[ 0]=0x03;Dt[ 1]=0x00;Dt[ 2]=0x00;Dt[ 3]=0x00;                           /* Destination Address */
   *((AiUInt16*)(&Dt[04])) =  BSWAP16_MAC(1200);                               /* VL */
   Dt[ 6]=0x02;Dt[ 7]=0x00;Dt[ 8]=0x00;Dt[ 9]=0x01;Dt[10]=0x21;Dt[11]=0x20;   /* Source Address */
   Dt[12]=0x08;Dt[13]=0x00;                                                   /* Type */
   /* IP */
   Dt[14]=0x45;Dt[15]=0x00;
   *((AiUInt16*)(&Dt[16])) = BSWAP16_MAC(us_FrameCount -19);                  /* Ip Total Length */
   Dt[18]=0x00;Dt[19]=0x00;Dt[20]=0x40;Dt[21]=0x00;
   Dt[22]=0x01;Dt[23]=0x11;
   Dt[24]=0x00;Dt[25]=0x00;                                                   /* Byte 24 and 25 = Ip Header Check sum set to 0 for correct calculation*/
   *((AiUInt32*)(&Dt[26])) = BSWAP32_MAC(ul_GenerateIp("10.1.33.1") );        /* Source Address */
   *((AiUInt32*)(&Dt[30])) = BSWAP32_MAC(ul_GenerateIp("224.224.1.44") );    /* Destinatin Address*/
   *((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
   /* UDP */
   *((AiUInt16*)(&Dt[34])) =  BSWAP16_MAC(2001);                              /* Src-UDP */
   *((AiUInt16*)(&Dt[36])) =  BSWAP16_MAC(2002);                              /* Dst-UDP */
   Dt[38]=0x00;Dt[39]=0x19;Dt[40]=0x00;Dt[41]=0x00;
   /* Data */
   sprintf((char*)&Dt[42],"F No. %d, %d Byte", uc_FrameNo, us_FrameCount); 

   printf("\n FdxCmdTxSubQueueWrite Frame %d ...", uc_FrameNo);
   if (FDX_OK != (r_RetVal =  FdxCmdTxSubQueueWrite (    ul_Handle,
                                                      paul_TxSubQueueHandle[ul_SubQueueHandleIndex],
                                                      1, 
                                                      sizeof(My_Frame), 
                                                      &My_Frame) ) )
      printf("FdxCmdTxSubQueueWrite failed!\n");
   else
      printf("FdxCmdTxSubQueueWrite done!\n");


   /* Fill SubQueue 2 Frame 2 #################################################################### */
   uc_FrameNo = 81;
   us_FrameCount = 128;

   for ( i = 0 ; i<1600; i++)
      My_Frame.uc_Data[i] = (unsigned char) uc_FrameNo;

   My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize              = us_FrameCount;
   My_Frame.x_Frame.x_FrameAttrib.uc_PayloadBufferMode      = FDX_TX_FRAME_PBM_STD;
   My_Frame.x_Frame.x_FrameAttrib.ul_BufferQueueHandle      = 0;
   
   Dt = &My_Frame.uc_Data[0];

   /* MAC */
   Dt[ 0]=0x03;Dt[ 1]=0x00;Dt[ 2]=0x00;Dt[ 3]=0x00;                           /* Destination Address */
   *((AiUInt16*)(&Dt[04])) =  BSWAP16_MAC(1210);                               /* VL */
   Dt[ 6]=0x02;Dt[ 7]=0x00;Dt[ 8]=0x00;Dt[ 9]=0x01;Dt[10]=0x21;Dt[11]=0x20;   /* Source Address */
   Dt[12]=0x08;Dt[13]=0x00;                                                   /* Type */
   /* IP */
   Dt[14]=0x45;Dt[15]=0x00;
   *((AiUInt16*)(&Dt[16])) = BSWAP16_MAC(us_FrameCount -19);                  /* Ip Total Length */
   Dt[18]=0x00;Dt[19]=0x00;Dt[20]=0x40;Dt[21]=0x00;
   Dt[22]=0x01;Dt[23]=0x11;
   Dt[24]=0x00;Dt[25]=0x00;                                                   /* Byte 24 and 25 = Ip Header Check sum set to 0 for correct calculation*/
   *((AiUInt32*)(&Dt[26])) = BSWAP32_MAC(ul_GenerateIp("10.1.33.1") );        /* Source Address */
   *((AiUInt32*)(&Dt[30])) = BSWAP32_MAC(ul_GenerateIp("224.224.1.44") );    /* Destinatin Address*/
   *((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
   /* UDP */
   *((AiUInt16*)(&Dt[34])) =  BSWAP16_MAC(2003);                              /* Src-UDP */
   *((AiUInt16*)(&Dt[36])) =  BSWAP16_MAC(2004);                              /* Dst-UDP */
   Dt[38]=0x00;Dt[39]=0x19;Dt[40]=0x00;Dt[41]=0x00;
   /* Data */
   sprintf((char*)&Dt[42],"F No. %d, %d Byte", uc_FrameNo, us_FrameCount); 

   printf("\n FdxCmdTxSubQueueWrite Frame %d ...", uc_FrameNo);
   if (FDX_OK != (r_RetVal =  FdxCmdTxSubQueueWrite (    ul_Handle,
                                                      paul_TxSubQueueHandle[ul_SubQueueHandleIndex],
                                                      1, 
                                                      sizeof(My_Frame), 
                                                      &My_Frame) ) )
     printf("FdxCmdTxSubQueueWrite failed!\n");
   else
      printf("FdxCmdTxSubQueueWrite done!\n");


   /* Fill SubQueue 2 Frame 3 #################################################################### */
   uc_FrameNo = 82;
   us_FrameCount = 257;

   for ( i = 0 ; i<1600; i++)
      My_Frame.uc_Data[i] = (unsigned char) uc_FrameNo;

   My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize              = us_FrameCount;
   My_Frame.x_Frame.x_FrameAttrib.uc_PayloadBufferMode      = FDX_TX_FRAME_PBM_STD;
   My_Frame.x_Frame.x_FrameAttrib.ul_BufferQueueHandle      = 0;
   
   Dt = &My_Frame.uc_Data[0];

   /* MAC */
   Dt[ 0]=0x03;Dt[ 1]=0x00;Dt[ 2]=0x00;Dt[ 3]=0x00;                           /* Destination Address */
   *((AiUInt16*)(&Dt[04])) =  BSWAP16_MAC(1220);                               /* VL */
   Dt[ 6]=0x02;Dt[ 7]=0x00;Dt[ 8]=0x00;Dt[ 9]=0x01;Dt[10]=0x21;Dt[11]=0x20;   /* Source Address */
   Dt[12]=0x08;Dt[13]=0x00;                                                   /* Type */
   /* IP */
   Dt[14]=0x45;Dt[15]=0x00;
   *((AiUInt16*)(&Dt[16])) = BSWAP16_MAC(us_FrameCount -19);                  /* Ip Total Length */
   Dt[18]=0x00;Dt[19]=0x00;Dt[20]=0x40;Dt[21]=0x00;
   Dt[22]=0x01;Dt[23]=0x11;
   Dt[24]=0x00;Dt[25]=0x00;                                                   /* Byte 24 and 25 = Ip Header Check sum set to 0 for correct calculation*/
   *((AiUInt32*)(&Dt[26])) = BSWAP32_MAC(ul_GenerateIp("10.1.33.1") );        /* Source Address */
   *((AiUInt32*)(&Dt[30])) = BSWAP32_MAC(ul_GenerateIp("224.224.1.64") );    /* Destinatin Address*/
   *((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
   /* UDP */
   *((AiUInt16*)(&Dt[34])) =  BSWAP16_MAC(2005);                              /* Src-UDP */
   *((AiUInt16*)(&Dt[36])) =  BSWAP16_MAC(2006);                              /* Dst-UDP */
   Dt[38]=0x00;Dt[39]=0x19;Dt[40]=0x00;Dt[41]=0x00;
   /* Data */
   sprintf((char*)&Dt[42],"F No. %d, %d Byte", uc_FrameNo, us_FrameCount); 

   printf("\n FdxCmdTxSubQueueWrite Frame %d ...", uc_FrameNo);
   if (FDX_OK != (r_RetVal =  FdxCmdTxSubQueueWrite (    ul_Handle,
                                                      paul_TxSubQueueHandle[ul_SubQueueHandleIndex],
                                                      1, 
                                                      sizeof(My_Frame), 
                                                      &My_Frame) ) )
      printf("FdxCmdTxSubQueueWrite failed!\n");
   else
      printf("FdxCmdTxSubQueueWrite done!\n");


   /* Fill SubQueue 2 Frame 4 #################################################################### */
   uc_FrameNo = 83;
   us_FrameCount = 499;

   for ( i = 0 ; i<1600; i++)
      My_Frame.uc_Data[i] = (unsigned char) uc_FrameNo;

   My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize              = us_FrameCount;
   My_Frame.x_Frame.x_FrameAttrib.uc_PayloadBufferMode      = FDX_TX_FRAME_PBM_STD;
   My_Frame.x_Frame.x_FrameAttrib.ul_BufferQueueHandle      = 0;
   
   Dt = &My_Frame.uc_Data[0];

   /* MAC */
   Dt[ 0]=0x03;Dt[ 1]=0x00;Dt[ 2]=0x00;Dt[ 3]=0x00;                           /* Destination Address */
   *((AiUInt16*)(&Dt[04])) =  BSWAP16_MAC(1230);                               /* VL */
   Dt[ 6]=0x02;Dt[ 7]=0x00;Dt[ 8]=0x00;Dt[ 9]=0x01;Dt[10]=0x21;Dt[11]=0x20;   /* Source Address */
   Dt[12]=0x08;Dt[13]=0x00;                                                   /* Type */
   /* IP */
   Dt[14]=0x45;Dt[15]=0x00;
   *((AiUInt16*)(&Dt[16])) = BSWAP16_MAC(us_FrameCount -19);                  /* Ip Total Length */
   Dt[18]=0x00;Dt[19]=0x00;Dt[20]=0x40;Dt[21]=0x00;
   Dt[22]=0x01;Dt[23]=0x11;
   Dt[24]=0x00;Dt[25]=0x00;                                                   /* Byte 24 and 25 = Ip Header Check sum set to 0 for correct calculation*/
   *((AiUInt32*)(&Dt[26])) = BSWAP32_MAC(ul_GenerateIp("10.1.33.1") );        /* Source Address */
   *((AiUInt32*)(&Dt[30])) = BSWAP32_MAC(ul_GenerateIp("224.224.1.74") );    /* Destinatin Address*/
   *((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
   /* UDP */
   *((AiUInt16*)(&Dt[34])) =  BSWAP16_MAC(2007);                              /* Src-UDP */
   *((AiUInt16*)(&Dt[36])) =  BSWAP16_MAC(2008);                              /* Dst-UDP */
   Dt[38]=0x00;Dt[39]=0x19;Dt[40]=0x00;Dt[41]=0x00;
   /* Data */
   sprintf((char*)&Dt[42],"F No. %d, %d Byte", uc_FrameNo, us_FrameCount); 

   printf("\n FdxCmdTxSubQueueWrite Frame %d ...", uc_FrameNo);
   if (FDX_OK != (r_RetVal =  FdxCmdTxSubQueueWrite (    ul_Handle,
                                                      paul_TxSubQueueHandle[ul_SubQueueHandleIndex],
                                                      1, 
                                                      sizeof(My_Frame), 
                                                      &My_Frame) ) )
      printf("FdxCmdTxSubQueueWrite failed!\n");
   else
      printf("FdxCmdTxSubQueueWrite done!\n");


   /* SubQueue 2 filled */      

   /* Fill SubQueue 3 Frame 1 #################################################################### */
   ul_SubQueueHandleIndex++;
   uc_FrameNo = 100;
   us_FrameCount = 999;

   for ( i = 0 ; i<1600; i++)
      My_Frame.uc_Data[i] = (unsigned char) uc_FrameNo;

   My_Frame.x_Frame.uc_FrameType                            = FDX_TX_FRAME_STD;
   My_Frame.x_Frame.x_FrameAttrib.uc_NetSelect              = FDX_TX_FRAME_BOTH;
   My_Frame.x_Frame.x_FrameAttrib.uc_ExternalStrobe         = FDX_DIS;
   My_Frame.x_Frame.x_FrameAttrib.uc_FrameStartMode         = FDX_TX_FRAME_START_PGWT;
   My_Frame.x_Frame.x_FrameAttrib.uc_PayloadBufferMode      = FDX_TX_FRAME_PBM_STD;

   My_Frame.x_Frame.x_FrameAttrib.uc_PayloadGenerationMode  = FDX_TX_FRAME_PGM_USER;
   My_Frame.x_Frame.x_FrameAttrib.uc_PreambleCount          = FDX_TX_FRAME_PRE_DEF;
   My_Frame.x_Frame.x_FrameAttrib.ul_BufferQueueHandle      = 0;
   My_Frame.x_Frame.x_FrameAttrib.ul_InterFrameGap          = 25;                         /* 25=1usec;*/
   My_Frame.x_Frame.x_FrameAttrib.ul_PacketGroupWaitTime    = 500;                        /* 500=0,5msec*/
   My_Frame.x_Frame.x_FrameAttrib.ul_PhysErrorInjection     = FDX_TX_FRAME_ERR_OFF;
   My_Frame.x_Frame.x_FrameAttrib.ul_Skew                   = 50;
   My_Frame.x_Frame.x_FrameAttrib.uw_SequenceNumberInit     = FDX_TX_FRAME_SEQ_INIT_AUTO;
   My_Frame.x_Frame.x_FrameAttrib.uw_SequenceNumberOffset   = FDX_TX_FRAME_SEQ_OFFS_AUTO;

   My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize              = us_FrameCount;
   
   Dt = &My_Frame.uc_Data[0];

   /* MAC */
   Dt[ 0]=0x03;Dt[ 1]=0x00;Dt[ 2]=0x00;Dt[ 3]=0x00;                           /* Destination Address */
   *((AiUInt16*)(&Dt[04])) =  BSWAP16_MAC(3333);                               /* VL */
   Dt[ 6]=0x02;Dt[ 7]=0x00;Dt[ 8]=0x00;Dt[ 9]=0x01;Dt[10]=0x21;Dt[11]=0x20;   /* Source Address */
   Dt[12]=0x08;Dt[13]=0x00;                                                   /* Type */
   /* IP */
   Dt[14]=0x45;Dt[15]=0x00;
   *((AiUInt16*)(&Dt[16])) = BSWAP16_MAC(us_FrameCount -19);                  /* Ip Total Length */
   Dt[18]=0x00;Dt[19]=0x00;Dt[20]=0x40;Dt[21]=0x00;
   Dt[22]=0x01;Dt[23]=0x11;
   Dt[24]=0x00;Dt[25]=0x00;                                                   /* Byte 24 and 25 = Ip Header Check sum set to 0 for correct calculation*/
   *((AiUInt32*)(&Dt[26])) = BSWAP32_MAC(ul_GenerateIp("10.1.33.1") );        /* Source Address */
   *((AiUInt32*)(&Dt[30])) = BSWAP32_MAC(ul_GenerateIp("224.224.1.44") );    /* Destinatin Address*/
   *((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
   /* UDP */
   *((AiUInt16*)(&Dt[34])) =  BSWAP16_MAC(3001);                              /* Src-UDP */
   *((AiUInt16*)(&Dt[36])) =  BSWAP16_MAC(3002);                              /* Dst-UDP */
   Dt[38]=0x00;Dt[39]=0x19;Dt[40]=0x00;Dt[41]=0x00;
   /* Data */
   sprintf((char*)&Dt[42],"F No. %d, %d Byte", uc_FrameNo, us_FrameCount); 

   printf("\n FdxCmdTxSubQueueWrite Frame %d ...", uc_FrameNo);
   if (FDX_OK != (r_RetVal =  FdxCmdTxSubQueueWrite (    ul_Handle,
                                                      paul_TxSubQueueHandle[ul_SubQueueHandleIndex],
                                                      1, 
                                                      sizeof(My_Frame), 
                                                      &My_Frame) ) )
      printf("FdxCmdTxSubQueueWrite failed!\n");
   else
      printf("FdxCmdTxSubQueueWrite done!\n");


   /* Fill SubQueue 3 Frame 2 #################################################################### */
   uc_FrameNo = 101;
   us_FrameCount = 1300;

   My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize              = us_FrameCount;
   My_Frame.x_Frame.x_FrameAttrib.uc_PayloadBufferMode      = FDX_TX_FRAME_PBM_STD;
   My_Frame.x_Frame.x_FrameAttrib.ul_BufferQueueHandle      = 0;
   
   Dt = &My_Frame.uc_Data[0];

   /* MAC */
   Dt[ 0]=0x03;Dt[ 1]=0x00;Dt[ 2]=0x00;Dt[ 3]=0x00;                           /* Destination Address */
   *((AiUInt16*)(&Dt[04])) =  BSWAP16_MAC(4444);                               /* VL */
   Dt[ 6]=0x02;Dt[ 7]=0x00;Dt[ 8]=0x00;Dt[ 9]=0x01;Dt[10]=0x21;Dt[11]=0x20;   /* Source Address */
   Dt[12]=0x08;Dt[13]=0x00;                                                   /* Type */
   /* IP */
   Dt[14]=0x45;Dt[15]=0x00;
   *((AiUInt16*)(&Dt[16])) = BSWAP16_MAC(us_FrameCount -19);                  /* Ip Total Length */
   Dt[18]=0x00;Dt[19]=0x00;Dt[20]=0x40;Dt[21]=0x00;
   Dt[22]=0x01;Dt[23]=0x11;
   Dt[24]=0x00;Dt[25]=0x00;                                                   /* Byte 24 and 25 = Ip Header Check sum set to 0 for correct calculation*/
   *((AiUInt32*)(&Dt[26])) = BSWAP32_MAC(ul_GenerateIp("10.1.33.1") );        /* Source Address */
   *((AiUInt32*)(&Dt[30])) = BSWAP32_MAC(ul_GenerateIp("224.224.1.44") );    /* Destinatin Address*/
   *((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
   /* UDP */
   *((AiUInt16*)(&Dt[34])) =  BSWAP16_MAC(3003);                              /* Src-UDP */
   *((AiUInt16*)(&Dt[36])) =  BSWAP16_MAC(3004);                              /* Dst-UDP */
   Dt[38]=0x00;Dt[39]=0x19;Dt[40]=0x00;Dt[41]=0x00;
   /* Data */
   sprintf((char*)&Dt[42],"F No. %d, %d Byte", uc_FrameNo, us_FrameCount); 

   printf("\n FdxCmdTxSubQueueWrite Frame %d ...", uc_FrameNo);
   if (FDX_OK != (r_RetVal =  FdxCmdTxSubQueueWrite (    ul_Handle,
                                                      paul_TxSubQueueHandle[ul_SubQueueHandleIndex],
                                                      1, 
                                                      sizeof(My_Frame), 
                                                      &My_Frame) ) )
      printf("FdxCmdTxSubQueueWrite failed!\n");
   else
      printf("FdxCmdTxSubQueueWrite done!\n");

   /* Setup Instruction  No. 3 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
#ifdef INSTRUCTION_ACYC_MYRK
   uc_InstrNo = 31;
   My_Frame.x_Frame.uc_FrameType                      = FDX_TX_FRAME_INSTR;
   memset(&My_Frame.x_Frame.x_InstrAttrib, 0, sizeof(TY_FDX_TX_INSTR_ATTRIB));
   My_Frame.x_Frame.x_InstrAttrib.uc_Code             = FDX_TX_FRAME_INSTR_ACYC_MARK;
   My_Frame.x_Frame.x_InstrAttrib.uc_Interrupt        = FDX_DIS;
   My_Frame.x_Frame.x_InstrAttrib.uc_NumOfSubQueues   = 0;
   My_Frame.x_Frame.x_InstrAttrib.uc_ActivSubQueue    = 0;
   My_Frame.x_Frame.x_InstrAttrib.aul_SubQueueHandle[0]  = 0;

   printf("\n FdxCmdTxQueueWrite Instr.  %d ...",    uc_InstrNo);
   if (FDX_OK != (r_RetVal = FdxCmdTxSubQueueWrite (    ul_Handle, 
                                                      paul_TxSubQueueHandle[ul_SubQueueHandleIndex],
                                                      1,
                                                      sizeof(My_Frame),
                                                      &My_Frame)))
       
      printf("\r\nFdxCmdTxQueueWrite() failed.");
#endif  
   return r_RetVal;
}

/***
 *
 *
 *
 *****/
AiReturn l_SendAcyclicGenericFrame(AiUInt32 ul_Handle)
{
AiReturn rRet;
struct my_Frame_tag {
   TY_FDX_TX_FRAME_HEADER x_Frame;
   AiUInt8 uc_Data[1600];
} My_Frame;
AiUInt8  *Dt;   /* was 58 */
AiUInt8 uc_FrameData = 0;
AiUInt16 us_FrameCount = 333;
int i;

   memset(&My_Frame.x_Frame, 0, sizeof(TY_FDX_TX_FRAME_HEADER) );

   /* Setup Acyclic Frame   +++++++++++++++++++++++++++++++++++++++++++++++++++++ */
   My_Frame.x_Frame.uc_FrameType                            = FDX_TX_FRAME_STD;
   My_Frame.x_Frame.x_FrameAttrib.uc_NetSelect              = FDX_TX_FRAME_BOTH;
   My_Frame.x_Frame.x_FrameAttrib.uc_ExternalStrobe         = FDX_ENA;
   My_Frame.x_Frame.x_FrameAttrib.uc_FrameStartMode         = FDX_TX_FRAME_START_PGWT;
   My_Frame.x_Frame.x_FrameAttrib.uc_PayloadBufferMode      = FDX_TX_FRAME_PBM_STD;
   My_Frame.x_Frame.x_FrameAttrib.uc_PayloadGenerationMode  = FDX_TX_FRAME_PGM_USER;
   My_Frame.x_Frame.x_FrameAttrib.uc_PreambleCount          = FDX_TX_FRAME_PRE_DEF;
   My_Frame.x_Frame.x_FrameAttrib.ul_BufferQueueHandle      = 0;
   My_Frame.x_Frame.x_FrameAttrib.ul_InterFrameGap          = 25;                         /* 25=1usec;*/
   My_Frame.x_Frame.x_FrameAttrib.ul_PacketGroupWaitTime    = 500;                        /* 500=0,5msec*/
   My_Frame.x_Frame.x_FrameAttrib.ul_PhysErrorInjection     = FDX_TX_FRAME_ERR_OFF;
   My_Frame.x_Frame.x_FrameAttrib.ul_Skew                   = 50;
   My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize              = 64;
   My_Frame.x_Frame.x_FrameAttrib.uw_SequenceNumberInit     = FDX_TX_FRAME_SEQ_INIT_AUTO;
   My_Frame.x_Frame.x_FrameAttrib.uw_SequenceNumberOffset   = FDX_TX_FRAME_SEQ_OFFS_AUTO;


   My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize              = us_FrameCount;
   My_Frame.x_Frame.x_FrameAttrib.uw_SequenceNumberInit     = FDX_TX_FRAME_SEQ_OFF;
   for ( i = 0 ; i<1600; i++)
      My_Frame.uc_Data[i] = (unsigned char) uc_FrameData++;

   Dt = &My_Frame.uc_Data[0];
   /* MAC*/
   Dt[ 0]=0x03;Dt[ 1]=0x00;Dt[ 2]=0x00;Dt[ 3]=0x00;                           /* Destination Address */
   *((AiUInt16*)(&Dt[04])) =  BSWAP16_MAC(1234);                              /* VL */
   Dt[ 6]=0x02;Dt[ 7]=0x00;Dt[ 8]=0x00;Dt[ 9]=0x01;Dt[10]=0x21;Dt[11]=0x20;   /* Source Address */
   Dt[12]=0x08;Dt[13]=0x00;                                                   /* Type */
   /* IP */
   Dt[14]=0x45;Dt[15]=0x00;
   *((AiUInt16*)(&Dt[16])) = BSWAP16_MAC(us_FrameCount -19);                  /* Ip Total Length */
   Dt[18]=0x00;Dt[19]=0x00;Dt[20]=0x40;Dt[21]=0x00;
   Dt[22]=0x01;Dt[23]=0x11;
   Dt[24]=0x00;Dt[25]=0x00;                                                   /* Byte 24 and 25 = Ip Header Check sum set to 0 for correct calculation*/
   *((AiUInt32*)(&Dt[26])) = BSWAP32_MAC(ul_GenerateIp("10.1.33.1") );        /* Source Address */
   *((AiUInt32*)(&Dt[30])) = BSWAP32_MAC(ul_GenerateIp("224.224.4.210") );      /* Destinatin Address*/
   *((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
   /* UDP */
   *((AiUInt16*)(&Dt[34])) =  BSWAP16_MAC(77);                                 /* Src-UDP */
   *((AiUInt16*)(&Dt[36])) =  BSWAP16_MAC(88);                                 /* Dst-UDP */
   Dt[38]=0x00;Dt[39]=0x19;Dt[40]=0x00;Dt[41]=0x00;
   /* Data */
   sprintf((char*)&Dt[42],"This as an acyclic Frame !!!"); 

   printf("\n FdxCmdTxQueueAcyclic Frame ...");
   if (FDX_OK != (rRet = FdxCmdTxQueueAcyclic (ul_Handle,sizeof(My_Frame),&My_Frame)))
      printf("\r\n FdxCmdTxQueueAcyclic() failed ERROR: %x", rRet);

   return rRet;
}


/***
 *
 *
 *
 *****/
void v_TxExtendedControlFunctions(AiUInt32 ul_Handle, AiUInt32 *paul_SubQueueHdl)
{
int iCh, finish = 0,lNoprompt;
AiReturn rRet;
AiUInt32 ul_Toggle_F = 0, ul_Toggle_I = 0, ul_Toggle_IFG = 0, ul_Toggle_PGWT = 0, ul_Toggle_SM = 0, ul_Toggle_INT = 0, ul_Toggle_SQ = 1;


/*typedef struct {
    AiUInt32 ul_SubQueueHandle;
    AiUInt32 ul_Index;
    AiUInt32 ul_ControlType;
    AiUInt32 ul_DisaEna;
    AiUInt32 ul_Size;
    AiUInt32 ul_IFG;
    AiUInt32 ul_PGW;
    AiUInt32 ul_PError;
    AiUInt32 ul_StartMode;
    AiUInt32 ul_DisaEnaInt;
    AiUInt32 ul_NextSubQueueIndex;
}*/ 
TY_FDX_TX_QUEUE_CONTROL x_TxQueueControl; 




   printf(" Extended Control Function overfiew :\r\n");
   printf(" Press '1' Insert Acyclic Frame\r\n");
   printf(" Press '2' Disable / Enable Frame \r\n");
   printf(" Press '3' Disable / Enable Insturction \r\n");
   printf(" Press '4' Modify Size and physical Error\r\n");
   printf(" Press '5' Modify IFG\r\n");
   printf(" Press '6' Modify PGW\r\n");
   printf(" Press '7' Reset Size Error\r\n");
   printf(" Press '8' Reset physical Errors\r\n");
   printf(" Press '9' Modyfy Start Mode\r\n");
   printf(" Press 'a' Disable / Enable Interrupt\r\n");
   printf(" Press 'b' Switch to next Sub Queue Index\r\n");
   printf(" Press 'c' Print Interrupt counters\r\n");
   printf(" Press 'd' Enable Print of Interrupt TimeTags\r\n");
   printf(" Press '' \r\n");
   printf(" Press 'e' for Exit :\r\n");

   do
   {
      iCh = GetConsoleCharacter();
      switch (iCh)
      {
         case 'e':
            finish = 1;
            break;

         case '1':
            printf(" \r\n");   
            rRet = l_SendAcyclicGenericFrame(ul_Handle);
            break;

         case '2':
            /* Disabel / Enable Frame */
            memset( &x_TxQueueControl, 0, sizeof(x_TxQueueControl) );
            x_TxQueueControl.ul_Index = 6;
            x_TxQueueControl.ul_ControlType = FDX_TX_CTL_ENDIS;

            if ( ul_Toggle_F == 0) {
               printf("\n\rDisabel Frame 6 \n\r");
               x_TxQueueControl.ul_DisaEna     = FDX_DIS;
            }
            else {
               printf("\n\rEnable Frame 6 \n\r");
               x_TxQueueControl.ul_DisaEna     = FDX_ENA;
            }
            if ( FDX_OK != (rRet = FdxCmdTxQueueControl( ul_Handle, &x_TxQueueControl)))
               printf("\r\n FdxCmdTxQueueControl() failed ERROR: %x", rRet);
            ul_Toggle_F++;
            ul_Toggle_F %=2;
            break;

         case '3':
            /* Disabel / Enable Instruction */
            memset( &x_TxQueueControl, 0, sizeof(x_TxQueueControl) );
            x_TxQueueControl.ul_Index = 5;
            x_TxQueueControl.ul_ControlType = FDX_TX_CTL_ENDIS;

            if ( ul_Toggle_I == 0) {
               printf("\n\rDisabel Instruction 5 \n\r");
               x_TxQueueControl.ul_DisaEna     = FDX_DIS;
            }
            else {
               printf("\n\rEnable Instruction 5 \n\r");
               x_TxQueueControl.ul_DisaEna     = FDX_ENA;
            }
            if ( FDX_OK != (rRet = FdxCmdTxQueueControl( ul_Handle, &x_TxQueueControl)))
               printf("\r\n FdxCmdTxQueueControl() failed ERROR: %x", rRet);
            ul_Toggle_I++;
            ul_Toggle_I %=2;
            break;

         case '4':
            /*  Modify Size and Physical Errors within one call */ 
            memset( &x_TxQueueControl, 0, sizeof(x_TxQueueControl) );
            x_TxQueueControl.ul_Index = 3;
            x_TxQueueControl.ul_ControlType = FDX_TX_CTL_SIZE | FDX_TX_CTL_PERROR;
            x_TxQueueControl.ul_SubQueueHandle = paul_SubQueueHdl[0];

            printf("\n\rSwitch Frame 3 in Sub Queue 1: Size physical error on");
            x_TxQueueControl.ul_Size = 400;
            x_TxQueueControl.ul_PError = FDX_TX_FRAME_ERR_CRC; 
            if ( FDX_OK != (rRet = FdxCmdTxQueueControl( ul_Handle, &x_TxQueueControl)))
               printf("\r\n FdxCmdTxQueueControl() failed ERROR: %x", rRet);

            break;

         case '5':
            /* Toggle Inter Frame Gap */
            memset( &x_TxQueueControl, 0, sizeof(x_TxQueueControl) );
            x_TxQueueControl.ul_Index = 6;
            x_TxQueueControl.ul_ControlType = FDX_TX_CTL_IFG;
            if ( ul_Toggle_IFG == 0) {
               printf("\n\rSwitch Frame 6 Interframe Gap to 3us\n\r");
               x_TxQueueControl.ul_IFG = 75;
            }
            else {
               printf("\n\rSwitch Frame 6 Interframe Gap back to 1us\n\r");
               x_TxQueueControl.ul_IFG = 25;
            }
            if ( FDX_OK != (rRet = FdxCmdTxQueueControl( ul_Handle, &x_TxQueueControl)))
               printf("\r\n FdxCmdTxQueueControl() failed ERROR: %x", rRet);

            ul_Toggle_IFG++;
            ul_Toggle_IFG %=2;
            break;

         case '6':
            /*  Change PGWT */
            memset( &x_TxQueueControl, 0, sizeof(x_TxQueueControl) );
            x_TxQueueControl.ul_Index = 6;
            x_TxQueueControl.ul_ControlType = FDX_TX_CTL_PGWT;
            if ( ul_Toggle_PGWT == 0) {
               printf("\n\rSwitch Frame 6 in Main Queue to PGWT = 1500us\n\r");
               x_TxQueueControl.ul_PGWT = 1500;
            }
            else {
               printf("\n\rSwitch Frame 6 in Main Queue to PGWT = 500us\n\r");
               x_TxQueueControl.ul_PGWT = 500;
            }
            if ( FDX_OK != (rRet = FdxCmdTxQueueControl( ul_Handle, &x_TxQueueControl)))
               printf("\r\n FdxCmdTxQueueControl() failed ERROR: %x", rRet);

            ul_Toggle_PGWT++;
            ul_Toggle_PGWT %=2;
            break;

         case '7':
            /* Correct Size to original value */
            memset( &x_TxQueueControl, 0, sizeof(x_TxQueueControl) );
            x_TxQueueControl.ul_Index = 3;
            x_TxQueueControl.ul_ControlType = FDX_TX_CTL_SIZE;
            x_TxQueueControl.ul_SubQueueHandle = paul_SubQueueHdl[0];

            printf("\n\rReset size error of Frame 3 in Sub Queue 1");
            x_TxQueueControl.ul_Size = 499;
            if ( FDX_OK != (rRet = FdxCmdTxQueueControl( ul_Handle, &x_TxQueueControl)))
               printf("\r\n FdxCmdTxQueueControl() failed ERROR: %x", rRet);

            break;

         case '8':
            /* Switch off physical Errors */
            memset( &x_TxQueueControl, 0, sizeof(x_TxQueueControl) );
            x_TxQueueControl.ul_Index = 3;
            x_TxQueueControl.ul_ControlType = FDX_TX_CTL_PERROR;
            x_TxQueueControl.ul_SubQueueHandle = paul_SubQueueHdl[0];

            printf("\n\rReset physocal Error of Frame 3 in Sub Queue 1");
            x_TxQueueControl.ul_PError = FDX_TX_FRAME_ERR_OFF; 
            if ( FDX_OK != (rRet = FdxCmdTxQueueControl( ul_Handle, &x_TxQueueControl)))
               printf("\r\n FdxCmdTxQueueControl() failed ERROR: %x", rRet);

            break;

         case '9':
            /* Toggle Start Mode */
            memset( &x_TxQueueControl, 0, sizeof(x_TxQueueControl) );
            x_TxQueueControl.ul_Index = 6;
            x_TxQueueControl.ul_ControlType = FDX_TX_CTL_SMODE;
            if ( ul_Toggle_SM == 0) {
               printf("\n\rSwitch Frame 6 in Main Queue to Start after Inter Frame Gap\n\r");
               x_TxQueueControl.ul_StartMode = FDX_TX_FRAME_START_IFG;
            }
            else {
               printf("\n\rSwitch Frame 6 in Main Queue to Start on PGWT\n\r");
               x_TxQueueControl.ul_StartMode = FDX_TX_FRAME_START_PGWT;
            }
            if ( FDX_OK != (rRet = FdxCmdTxQueueControl( ul_Handle, &x_TxQueueControl)))
               printf("\r\n FdxCmdTxQueueControl() failed ERROR: %x", rRet);

            ul_Toggle_SM++;
            ul_Toggle_SM %=2;
            break;

         case 'a':
            /* Disabel / Enable Interrup */
            memset( &x_TxQueueControl, 0, sizeof(x_TxQueueControl) );
            x_TxQueueControl.ul_Index = 8;
            x_TxQueueControl.ul_ControlType = FDX_TX_CTL_ENDISINT;

            if ( ul_Toggle_INT == 0) {
               printf("\n\rDisabel Interrupt on Frame 8 \n\r");
               x_TxQueueControl.ul_DisaEnaInt = FDX_DIS;
            }
            else {
               printf("\n\rEnable Interrupt on Frame 8 \n\r");
               x_TxQueueControl.ul_DisaEnaInt = FDX_ENA;
            }
            if ( FDX_OK != (rRet = FdxCmdTxQueueControl( ul_Handle, &x_TxQueueControl)))
               printf("\r\n FdxCmdTxQueueControl() failed ERROR: %x", rRet);
            
            ul_Toggle_INT++;
            ul_Toggle_INT %=2;
            break;
            break;

         case 'b':
            memset( &x_TxQueueControl, 0, sizeof(x_TxQueueControl) );
            x_TxQueueControl.ul_Index = 3;
            x_TxQueueControl.ul_ControlType = FDX_TX_CTL_SUBQUE;
            printf("\n\rSwitch Instruction Frame 3 in Main Queue to second SubQueue");
            x_TxQueueControl.ul_NextSubQueueIndex = ul_Toggle_SQ;
            
            if ( FDX_OK != (rRet = FdxCmdTxQueueControl( ul_Handle, &x_TxQueueControl)))
               printf("\r\n FdxCmdTxQueueControl() failed ERROR: %x", rRet);

            ul_Toggle_SQ++;
            ul_Toggle_SQ %=3;
            break;

         case '\n':
         case '\r':
            lNoprompt = 1;
            break;
         case '?':
         default:
            printf(" Extended Control Function overfiew :\r\n");
            printf(" Press '1' Insert Acyclic Frame\r\n");
            printf(" Press '2' Disable / Enable Frame \r\n");
            printf(" Press '3' Disable / Enable Insturction \r\n");
            printf(" Press '4' Modify Size and physical Error\r\n");
            printf(" Press '5' Modify IFG\r\n");
            printf(" Press '6' Modify PGW\r\n");
            printf(" Press '7' Reset Size Error\r\n");
            printf(" Press '8' Reset physical Errors\r\n");
            printf(" Press '9' Modyfy Start Mode\r\n");
            printf(" Press 'a' Disable / Enable Interrupt\r\n");
            printf(" Press 'b' Switch to next Sub Queue Index\r\n");
            printf(" Press 'c' Print Interrupt counters\r\n");
            printf(" Press 'd' Enable Print of Interrupt TimeTags\r\n");
            printf(" Press '' \r\n");
            printf(" Press 'e' for Exit :\r\n");
            break;
      }
      if (lNoprompt)
         lNoprompt=0;
      else
         printf("e[xit]\r\n>");

   } while (finish == 0);
   
}

void v_TxExtendedStartStopTest(AiUInt32 ul_Handle)
{
   int iCh, finish = 0,lNoprompt, ii;
   AiReturn rRet;

   TY_FDX_TX_CTRL x_TxControl;
   TY_FDX_TX_STATUS x_TxStatus;

   x_TxControl.ul_Count    = 0;               /* 0 = cyclic transmission */
   x_TxControl.e_StartMode = FDX_STOP;
   x_TxControl.e_ExtendedStopMode = FDX_ESTOP_NOT_USED;




   printf(" Extended Start Stop Test :\r\n");
   printf(" Press '1' Stop Tx\r\n");
   printf(" Press '2' Restart TX \r\n");
   printf(" Press '3' Stop/ Start several Times \r\n");
   printf(" Press '' \r\n");
   printf(" Press 'e' for Exit :\r\n");

   do
   {
      iCh = GetConsoleCharacter();
      switch (iCh)
      {
      case 'e':
         finish = 1;
         break;

      case '1':
         printf(" Stop Tx and get Status\r\n");   

         x_TxControl.e_StartMode = FDX_STOP;

         rRet = FdxCmdTxControl(ul_Handle, &x_TxControl);
         if(FDX_OK != rRet)
            printf ("\r\n Problem with TxControl! ");

         rRet = FdxCmdTxStatus(ul_Handle, &x_TxStatus);
         if(FDX_OK != rRet)
            printf ("\r\n Problem with TxControl! ");
         else
            printf("\r\n Mode: %d Status: %d TxFrames %d\n", x_TxStatus.ul_TransmitMode, (int) x_TxStatus.e_Status, x_TxStatus.ul_Frames );

         break;

      case '2':
         printf(" Start Tx and get Status\r\n");   
         x_TxControl.e_StartMode = FDX_START;

         rRet = FdxCmdTxControl(ul_Handle, &x_TxControl);
         if(FDX_OK != rRet)
            printf ("\r\n Problem with TxControl! ");

         rRet = FdxCmdTxStatus(ul_Handle, &x_TxStatus);
         if(FDX_OK != rRet)
            printf ("\r\n Problem with TxControl! ");
         else
            printf("\r\n Mode: %d Status: %d TxFrames %d\n", x_TxStatus.ul_TransmitMode, (int) x_TxStatus.e_Status, x_TxStatus.ul_Frames );

         break;

      case '3':
         printf(" Stop / Start Tx and get Status 100 times \r\n");   

         for (ii=0; ii<90; ii++)
         {

            x_TxControl.e_StartMode = FDX_STOP;

            rRet = FdxCmdTxControl(ul_Handle, &x_TxControl);
            if(FDX_OK != rRet)
               printf ("\r\n Problem with TxControl! ");

            rRet = FdxCmdTxStatus(ul_Handle, &x_TxStatus);
            if( (FDX_OK != rRet) && (FDX_STAT_STOP != x_TxStatus.e_Status) )
            {
               printf ("\r\n Problem with TxControl! ");
               printf("\r\n Mode: %d Status: %d TxFrames %d\n", x_TxStatus.ul_TransmitMode, (int) x_TxStatus.e_Status, x_TxStatus.ul_Frames );
            }

            x_TxControl.e_StartMode = FDX_START;

            rRet = FdxCmdTxControl(ul_Handle, &x_TxControl);
            if(FDX_OK != rRet)
               printf ("\r\n Problem with TxControl! ");

            rRet = FdxCmdTxStatus(ul_Handle, &x_TxStatus);
            if( (FDX_OK != rRet) && (FDX_STAT_RUN != x_TxStatus.e_Status) )
            {
               printf ("\r\n Problem with TxControl! ");
               printf("\r\n Mode: %d Status: %d TxFrames %d\n", x_TxStatus.ul_TransmitMode, (int) x_TxStatus.e_Status, x_TxStatus.ul_Frames );
            }

         }

         for (ii=0; ii<10; ii++)
         {

            x_TxControl.e_StartMode = FDX_STOP;

            rRet = FdxCmdTxControl(ul_Handle, &x_TxControl);
            if(FDX_OK != rRet)
               printf ("\r\n Problem with TxControl! ");

            rRet = FdxCmdTxStatus(ul_Handle, &x_TxStatus);
            if(FDX_OK != rRet)
               printf ("\r\n Problem with TxControl! ");
            else
               printf("\r\n Mode: %d Status: %d TxFrames %d\n", x_TxStatus.ul_TransmitMode, (int) x_TxStatus.e_Status, x_TxStatus.ul_Frames );

            x_TxControl.e_StartMode = FDX_START;

            rRet = FdxCmdTxControl(ul_Handle, &x_TxControl);
            if(FDX_OK != rRet)
               printf ("\r\n Problem with TxControl! ");

            rRet = FdxCmdTxStatus(ul_Handle, &x_TxStatus);
            if(FDX_OK != rRet)
               printf ("\r\n Problem with TxControl! ");
            else
               printf("\r\n Mode: %d Status: %d TxFrames %d\n", x_TxStatus.ul_TransmitMode, (int) x_TxStatus.e_Status, x_TxStatus.ul_Frames );

         }


         break;


      case '\n':
      case '\r':
         lNoprompt = 1;
         break;
      case '?':
      default:
         printf(" Extended Start Stop Test :\r\n");
         printf(" Press '1' Stop Tx\r\n");
         printf(" Press '2' Restart TX \r\n");
         printf(" Press '3' Stop/ Start several Times \r\n");
         printf(" Press '' \r\n");
         printf(" Press 'e' for Exit :\r\n");
         break;
      }
      if (lNoprompt)
         lNoprompt=0;
      else
         printf("e[xit]\r\n>");

   } while (finish == 0);

}



/* EOF */


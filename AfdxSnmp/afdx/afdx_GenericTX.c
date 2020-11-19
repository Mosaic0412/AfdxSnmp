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
Test functions for generic transmit mode.

-------------------------------------------------------------------------------

******************************************************************************/


/******************************************************************************
*  Includes
******************************************************************************/
#include <afdx/afdx_SampleUtils.h>
#include <afdx/afdx_GenericTX.h>
#include <AiFdx_def.h>
#include <math.h>

AiUInt32 ul_PortConfig;


/******************************************************************************
*  local prototypes
******************************************************************************/
AiInt32 l_SetupDataBufferQueues(AiUInt32 ul_Handle, TY_FDX_FW_BUF_HDL *pax_TxBufferQueueHandle);
AiInt32 l_SetupTxSubQueues(AiUInt32 ul_Handle, TY_FDX_FW_BUF_HDL *pax_TxBufferQueueHandle, AiUInt32 *paul_TxSubQueueHandle);
void v_TxExtendedControlFunctions(AiUInt32 ul_Handle, AiUInt32 *paul_SubQueueHdl);
void v_TxExtendedStartStopTest(AiUInt32 ul_Handle);
AiInt32 l_SetTxQueueExtended(AiUInt32 ul_Handle, TY_FDX_FW_BUF_HDL *pax_TxBufferQueueHandle, AiUInt32 *paul_TxSubQueueHandle, AiUInt32 ul_NumOfBufferQueues);

/*
##############################################################################
#  Start of Code
##############################################################################
*/



/****************************************************************************
Function    : l_SetTxStaticRegisters
-----------------------------------------------------------------------------
Create      : 1.01.2002 rh,AIM
-----------------------------------------------------------------------------

Inputs      : -

Outputs     : -

Description : Testfunction To set the TX static Registers.

*****************************************************************************/
AiInt32 l_SetTxStaticRegisters(AiUInt32 ul_Handle)
{
	int i;

	TY_FDX_TX_STATIC_REGS x_TxStaticRegs;

	fprintf(stderr, "\n Setup Static Registers... ");

	/* MAC */

	x_TxStaticRegs.x_TxStaticRegsMAC.uc_MACDest2 = 0x22;
	x_TxStaticRegs.x_TxStaticRegsMAC.uc_MACDest3 = 0x33;
	x_TxStaticRegs.x_TxStaticRegsMAC.uc_MACDest4 = 0x44;
	x_TxStaticRegs.x_TxStaticRegsMAC.uc_MACDest5 = 0x55;
	x_TxStaticRegs.x_TxStaticRegsMAC.uc_MACSrc0 = 0x00;
	x_TxStaticRegs.x_TxStaticRegsMAC.uc_MACSrc3 = 0x33;
	x_TxStaticRegs.x_TxStaticRegsMAC.uc_MACSrc4 = 0x44;
	x_TxStaticRegs.x_TxStaticRegsMAC.uc_MACSrc5 = 0x55;
	x_TxStaticRegs.x_TxStaticRegsMAC.uw_MACLengthType = 0x1234;

	/* IP */
	x_TxStaticRegs.x_TxStaticRegsIP.uc_IPCtrl = 1;

	x_TxStaticRegs.x_TxStaticRegsIP.uc_IPIHL = 2;
	x_TxStaticRegs.x_TxStaticRegsIP.uc_IPProtocol = 3;
	x_TxStaticRegs.x_TxStaticRegsIP.uc_IPTTLive = 4;
	x_TxStaticRegs.x_TxStaticRegsIP.uc_IPTypeSrv = 5;
	x_TxStaticRegs.x_TxStaticRegsIP.uc_IPVersion = 6;
	x_TxStaticRegs.x_TxStaticRegsIP.ul_IPDest = 0x11223344;
	x_TxStaticRegs.x_TxStaticRegsIP.ul_IPSrc = 0x55667788;
	x_TxStaticRegs.x_TxStaticRegsIP.uw_IPFrag = 0xBBAA;
	x_TxStaticRegs.x_TxStaticRegsIP.uw_IPFragOffs = 0x5544;
	x_TxStaticRegs.x_TxStaticRegsIP.uw_IPHeaderChkSum = 0x1234;
	x_TxStaticRegs.x_TxStaticRegsIP.uw_IPTotalLength = 0x5678;

	/* UDP */
	x_TxStaticRegs.x_TxStaticRegsUDP.uw_UDPChkSum = 0x4321;
	x_TxStaticRegs.x_TxStaticRegsUDP.uw_UDPDest = 0x9876;
	x_TxStaticRegs.x_TxStaticRegsUDP.uw_UDPLength = 0xABCD;
	x_TxStaticRegs.x_TxStaticRegsUDP.uw_UDPSrc = 0x5432;
	for (i = 0; i < 22; i++)
		x_TxStaticRegs.x_TxStaticRegsUDP.uc_UDPPayload[i] = i + 1;

	AIM_ASSERT(FdxCmdTxStaticRegsControl(ul_Handle, &x_TxStaticRegs));

	fprintf(stderr, " ...Done\n\n");

	return(FDX_OK);
}

//16->10
long ChangedToD(int i, int j, char *remain)
{
	int temp = 0;
	for (int t = i; t <= j; t++) //按权展开
	{
		int dt = 0;
		if (remain[t] >= 0 && remain[t] <= '9')
			temp += (remain[t]-'0') * pow(16, j - t);
		else{
			switch (remain[t]){
			case'a':
			case'A':dt = 10; break;
			case'b':
			case'B':dt = 11; break;
			case'c':
			case'C':dt = 12; break;
			case'd':
			case'D':dt = 13; break;
			case'e':
			case'E':dt = 14; break;
			case'f':
			case'F':dt = 15; break;
			}
			temp += dt * pow(16, j - t);
		}	
	}
	return temp;
}
//10->16
void ChangedTo0x(long num, char *remain)
{
	int i = 0, j = 0;
	int temp;
	do
	{
		temp = num % 16; //取余
		num /= 16;
		if (temp >= 10)
			remain[i++] = temp - 10 + 'a'; //任意进制为大于基数大于10的进制 例如，十六进制
		else
			remain[i++] = temp + '0';
	} while (num);
	for (j = 0; j < i / 2; j++)
	{
		temp = remain[j];
		remain[j] = remain[i - j - 1];
		remain[i - j - 1] = temp;
	}
}

/****************************************************************************
Function    : l_SetAFDXTxQueue
-----------------------------------------------------------------------------
Create      : 10.11.2005 rh,AIM
-----------------------------------------------------------------------------

Inputs      : -

Outputs     : -

Description : Testfunction for Init AFDX TX Port and create AFDX TX Queue.

*****************************************************************************/

AiInt32 l_SetAFDXTxQueue(AiUInt32 ul_Handle, afdx_netsnmp_address_list *address, int *snmp_data, AiUInt len){
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
	AiUInt8  Dt[1600];   /* was 58 */
	AiUInt8  uc_FrameNo = 0;
	AiUInt16 us_FrameCount;
	unsigned char MacDbuffer[17];
	unsigned char MacSbuffer[17];
	unsigned char IPSbuffer[15];
	unsigned char IPDbuffer[15];

	printf("\n FdxCmdTxPortInit...");

	x_PortInitIn.ul_PortMap = 1;
	AIM_ASSERT(FdxCmdTxPortInit(ul_Handle, &x_PortInitIn, &x_PortInitOut));
	/*
	printf("\n FdxCmdTxTrgLineControl...");
	x_TrgLineCtrl.ul_TrgInLine  = FDX_STROBE_LINE_1;
	x_TrgLineCtrl.ul_TrgOutLine = FDX_STROBE_LINE_1;

	AIM_ASSERT( FdxCmdTxTrgLineControl ( ul_Handle1, &x_TrgLineCtrl));
	*/
	printf("\n FdxCmdTxModeControl...");
	x_TxModeControl.ul_TransmitMode = FDX_TX_GENERIC;
	AIM_ASSERT(FdxCmdTxModeControl(ul_Handle, &x_TxModeControl));

	printf("\n FdxCmdTxQueueCreate...");
	x_TxQueueCreate.ul_QueueSize = 0;
	AIM_ASSERT(FdxCmdTxQueueCreate(ul_Handle, &x_TxQueueCreate, &x_TxQueueInfo));

	memset(&My_Frame.x_Frame, 0, sizeof(TY_FDX_TX_FRAME_HEADER));
	My_Frame.x_Frame.uc_FrameType = FDX_TX_FRAME_STD;
	My_Frame.x_Frame.x_FrameAttrib.uc_NetSelect = FDX_TX_FRAME_BOTH;
	My_Frame.x_Frame.x_FrameAttrib.uc_ExternalStrobe = FDX_DIS;
	My_Frame.x_Frame.x_FrameAttrib.uc_FrameStartMode = FDX_TX_FRAME_START_PGWT;
	My_Frame.x_Frame.x_FrameAttrib.uc_PayloadBufferMode = FDX_TX_FRAME_PBM_STD;

	My_Frame.x_Frame.x_FrameAttrib.uc_PayloadGenerationMode = FDX_TX_FRAME_PGM_USER;
	My_Frame.x_Frame.x_FrameAttrib.uc_PreambleCount = FDX_TX_FRAME_PRE_DEF;
	My_Frame.x_Frame.x_FrameAttrib.ul_BufferQueueHandle = 0;
	My_Frame.x_Frame.x_FrameAttrib.ul_InterFrameGap = 25;                         /* 25=1usec;*/
	My_Frame.x_Frame.x_FrameAttrib.ul_PacketGroupWaitTime = 500;                        /* 500=0,5msec*/
	My_Frame.x_Frame.x_FrameAttrib.ul_PhysErrorInjection = FDX_TX_FRAME_ERR_OFF;
	My_Frame.x_Frame.x_FrameAttrib.ul_Skew = 50;
	My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize = 90;
	My_Frame.x_Frame.x_FrameAttrib.uw_SequenceNumberInit = FDX_TX_FRAME_SEQ_INIT_AUTO;
	My_Frame.x_Frame.x_FrameAttrib.uw_SequenceNumberOffset = FDX_TX_FRAME_SEQ_OFFS_AUTO;
	// My_Frame.x_Frame.x_FrameAttrib.uc_TxIntEnable            = 3 /*FDX_ENA with Time Tag*/;

	// Setup Frame No. 1
	strcpy(MacDbuffer, address->DestinatinMac); //156
	strcpy(MacSbuffer, address->SourceMac);
	strcpy(IPSbuffer, address->SourceIP);
	strcpy(IPDbuffer, address->DestinatinIP);
	uc_FrameNo++;
	us_FrameCount = 90;
	My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize = us_FrameCount;
	for (i = 0; i < 1600; i++)
		My_Frame.uc_Data[i] = (unsigned char)uc_FrameNo;

	/* MAC*/
	int j = 0;
	for (int i = 0; i < 17; i = i + 3, j++)
	{
		Dt[j] = ChangedToD(i, i + 1, MacDbuffer);
	}
	//Dt[0] = 0x02; Dt[1] = 0x00; Dt[2] = 0x00; //固定域
	//Dt[3] = 0x68; Dt[4] = 0x00; 
	//Dt[5] = 0x20;

	for (int i = 0; i < 17; i = i + 3, j++)
	{
		Dt[j] = ChangedToD(i, i + 1, MacSbuffer);
	}

	Dt[12] = 0x08; Dt[13] = 0x00;                                                   /* Type */
	/* IP */
	Dt[14] = 0x45; Dt[15] = 0x00;
	Dt[16] = 0x00; Dt[17] = 0x46;                           /* Ip Total Length */
	*((AiUInt16*)(&Dt[16])) = BSWAP16_MAC(us_FrameCount - 19);
	Dt[18] = 0x07; Dt[19] = 0x41; Dt[20] = 0x00; Dt[21] = 0x00;
	Dt[22] = 0x80; Dt[23] = 0x11;
	Dt[24] = 0x00; Dt[25] = 0x00;                           /* Byte 24 and 25 = Ip Header Check sum set to 0 for correct calculation*/

	//ip -> ip
	int IPnumber;
	for (int i = 0, j = -1; i < 4; i++)
	{
		IPnumber = 0;
		while (IPSbuffer[++j] != '.' && IPSbuffer[j] != NULL)
		{
			IPnumber = IPnumber * 10 + (IPSbuffer[j] - '0');
		}

		//printf("%d\n", IPnumber);
		Dt[26 + i] = IPnumber;
	}
	//Dt[26] = 0xc0; Dt[27] = 0xa8; Dt[28] = 0x01; Dt[29] = 0x63;   /* Source Address */
	for (int i = 0, j = -1; i < 4; i++)
	{
		IPnumber = 0;
		while (IPDbuffer[++j] != '.' && IPDbuffer[j] != NULL)
		{
			IPnumber = IPnumber * 10 + (IPDbuffer[j] - '0');
		}
		Dt[30 + i] = IPnumber;
	}
	//Dt[30] = 0xc0; Dt[31] = 0xa8; Dt[32] = 0x01; Dt[33] = 0x9c;   /* Destinatin Address*/
	*((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
	/* UDP */
	Dt[34] = 0x00; Dt[35] = 0x01; Dt[36] = 0x00; Dt[37] = 0xa1;   Dt[38] = 0x00; Dt[39] = len + 8; Dt[40] = 0x00; Dt[41] = 0x00;

	/* Data
	//---AFDX Payload
	Dt[42] = 0x30; Dt[43] = len - 2;//snmp, length
	Dt[44] = 0x02; Dt[45] = 0x01;
	switch (data->version){
	case 0:Dt[46] = 0x00; //version-v1
	break;
	case 2:Dt[46] = 0x01; //version-v2c
	break;
	case 3:Dt[46] = 0x02; //version-v3
	break;
	}
	switch (*data->community){
	case 'p':Dt[47] = 0x04; Dt[48] = 0x06; //community, length
	Dt[49] = 0x70; Dt[50] = 0x75; Dt[51] = 0x62; Dt[52] = 0x6c; Dt[53] = 0x69; Dt[54] = 0x63; //public
	break;
	}

	Dt[55] = 0xa0; Dt[56] = 0x1b; //get-request, length:0x1b
	Dt[57] = 0x02; Dt[58] = 0x02; //request-id,length

	char remain[4] = "";
	int temp, Dt59 = 0, Dt60 = 0;
	ChangedTo0x(data->reqid, remain);

	Dt[59] = ChangedToD(0, 1, remain);
	Dt[60] = ChangedToD(2, 3, remain); //request-id

	Dt[61] = 0x02; Dt[62] = 0x01; Dt[63] = 0x00; //error-status
	Dt[64] = 0x02; Dt[65] = 0x01; Dt[66] = 0x00; //error-index

	Dt[67] = 0x30; Dt[68] = 0x0f; //variable-binding编码方式, length
	Dt[69] = 0x30; Dt[70] = 0x0d; //variable-1编码, length
	Dt[71] = 0x06; Dt[72] = data->variables->name_length - 1; //variable类型, length

	int length = data->variables->name_length;
	int oid[128];
	for (int i = 0; i < length; i++){
	oid[i] = data->variables->name_loc[i];
	}
	Dt[73] = oid[0] * 40 + oid[1];
	int l;
	for (l = 2; l < length; l++){
	if (oid[l] < 128)
	Dt[72 + l] = oid[l];
	}

	//Dt[73] = 0x2b; Dt[74] = 0x06; Dt[75] = 0x01; Dt[76] = 0x02; Dt[77] = 0x01;
	//Dt[78] = 0x19; Dt[79] = 0x01; Dt[80] = 0x02; Dt[81] = 0x00; //1.3.6.1.2.1.25.1.2.0
	Dt[72 + (l++)] = 0x05; Dt[72 + (l++)] = 0x00;
	*/

	for (int i = 0; i < len; i++){
		Dt[42 + i] = *(snmp_data)++;
	}

	for (i = 0; i < 42 + len; i++){
		My_Frame.uc_Data[i] = (unsigned char)Dt[i];
	}

	printf("\n FdxCmdTxQueueWrite Frame %d ...", uc_FrameNo);
	AIM_ASSERT(FdxCmdTxQueueWrite(ul_Handle, FDX_TX_FRAME_HEADER_GENERIC, 1, sizeof(My_Frame), &My_Frame));

	return(FDX_OK);
}

/****************************************************************************
Function    : l_TransmitAFDXSetup
-----------------------------------------------------------------------------
Create      :
-----------------------------------------------------------------------------

Inputs      : -

Outputs     : -

Description : Testfunction for AFDX TX Setup (with Sub Queues)

*****************************************************************************/
AiInt32 l_TransmitAFDXSetup(AiUInt32 ul_Handle1, AiUInt32 ul_Handle2, afdx_netsnmp_address_list *address, int *snmp_data, AiUInt len)
{
	AiUInt32 ul_Cycles = 0;
	AiUInt32 ul_ii;
	char c_Buf[32];
	AiChar Sel[100];
	TY_FDX_TRG_LINE_CTRL x_TrgLineCtrl;
	TY_FDX_FW_BUF_HDL ax_DataBufferQueueHandle[4];
	AiUInt32 aul_TxSubQueueHandle[4];
	AiUInt32 ul_BytesRead;

	for (ul_ii = 0; ul_ii < 4; ul_ii++)
	{
		ax_DataBufferQueueHandle[ul_ii].ul_Handle = 0;
		aul_TxSubQueueHandle[ul_ii] = 0;
	}

	/*
	printf("\n Additinal Information:");
	printf("\n To specify the number of transmittin cycles use  p <NoOfCycles> ");
	printf("\n To specify generate extended Frame setup         p <NoOfCycles> e ");
	printf("\n To specify generate fragmented frames use        p <NoOfCycles> f ");
	printf("\n (NoOfCycles = 0 means cyclic transmission");
	*/

	/* Setup a Trigger LIne */

	x_TrgLineCtrl.ul_TrgInLine = FDX_STROBE_LINE_1;
	x_TrgLineCtrl.ul_TrgOutLine = FDX_STROBE_LINE_1;

	FdxCmdTxTrgLineControl(ul_Handle1, &x_TrgLineCtrl);

	//if (fgets(c_Buf, sizeof((c_Buf)), stdin) != NULL){
	
		// parse the input string 
		sscanf(c_Buf, "%d %1s", &ul_Cycles, Sel);
		ul_Cycles = 1;
		if (ul_Cycles < 0)
		{
			printf("\n Invalid number of cycles.");
			return (FDX_ERR);
		}

		if ((Sel[0] == 'f') || (Sel[0] == 'F'))
		{
			printf("\n Fragmented Frame Setup");
			l_SetTxQueueFragmented(ul_Handle1);
			printf("\n ..");
			if (ul_Handle2 != 0)
			{
				l_SetTxQueueFragmented(ul_Handle2);
				printf("\n ..");
			}
		}
		else if ((Sel[0] == 'e') || (Sel[0] == 'E'))
		{
			printf("\n Frame Setup");
			l_SetTxQueueExtended(ul_Handle1, ax_DataBufferQueueHandle, aul_TxSubQueueHandle, 4);
			printf("\n ..");

			if (ul_Handle2 != 0)
			{
				l_SetTxQueueExtended(ul_Handle2, ax_DataBufferQueueHandle, aul_TxSubQueueHandle, 0);
				printf("\n ..");
			}
		}
		else
		{
			printf("\n Frame Setup (Basic)");
			l_SetAFDXTxQueue(ul_Handle1, address, snmp_data, len);
			printf("\n ..");

			if (ul_Handle2 != 0)
			{
				l_SetAFDXTxQueue(ul_Handle2, address, snmp_data, len);
				printf("\n ..");
			}
		}
		if (ul_Cycles == 0)
			printf("\n Cyclic TX");
		else
			printf("\n Number of TX Cycles:%d", ul_Cycles);

		l_StartTxGetStatus(ul_Handle1, ul_Handle2, ul_Cycles, aul_TxSubQueueHandle);

		for (ul_ii = 0; ul_ii < 4; ul_ii++)
		{
			if (ax_DataBufferQueueHandle[ul_ii].ul_Handle != 0)
			{
				FdxCmdTxBufferQueueRead(ul_Handle1,
					ax_DataBufferQueueHandle[ul_ii],
					FDX_TX_BUF_QUEUE_ACT,
					0,
					32,
					c_Buf,
					&ul_BytesRead);
				// delete third queue 
				if (FDX_OK != FdxCmdTxBufferQueueFree(ul_Handle1, ax_DataBufferQueueHandle[ul_ii]))
					printf("FdxCmdTxBufferQueueFree failed!\n");
				else
				{
					printf("FdxCmdTxBufferQueueFree Que deallocates successfully\n");
					ax_DataBufferQueueHandle[ul_ii].ul_Handle = 0;
				}
			}
			if (0 != aul_TxSubQueueHandle[ul_ii])
			{
				if (FDX_OK != FdxCmdTxSubQueueDelete(ul_Handle1, aul_TxSubQueueHandle[ul_ii]))
					printf("FdxCmdTxSubQueueDelete failed!\n");
				else
					printf("FdxCmdTxSubQueueDelete done!\n");
			}

		}
	/*}
	else
		printf("\n Error on Input");*/
	
	return FDX_ON;
}

/****************************************************************************
Function    : l_StartTxGetStatus
-----------------------------------------------------------------------------
Create      : 19.03.2009 RH/TJ,AIM
-----------------------------------------------------------------------------

Inputs      : -

Outputs     : -

Description : Testfunction for FdxCmdTxStatus (with Sub Queues).

*****************************************************************************/
AiInt32 l_StartTxGetStatus(AiUInt32 ul_Handle1, AiUInt32 ul_Handle2, AiUInt32 ul_Cycles, AiUInt32 *paul_SubQueueHdl)
{
	TY_FDX_TX_CTRL x_TxControl;
	TY_FDX_TX_STATUS x_TxStatus;

	AiUInt16 finish = 0;

	AIM_ASSERT(FdxCmdTxStatus(ul_Handle1, &x_TxStatus));

	if (ul_PortConfig == FDX_REDUNDANT)
		printf("\r\n Mode: %d Status: %d TxFrames PortA: %d PortB: %d \n", x_TxStatus.ul_TransmitMode, (int)x_TxStatus.e_Status, x_TxStatus.ul_Frames, x_TxStatus.ul_FramesPortB);
	else
		printf("\r\n Mode: %d Status: %d TxFrames %d\n", x_TxStatus.ul_TransmitMode, (int)x_TxStatus.e_Status, x_TxStatus.ul_Frames);

	if (ul_Handle2 != 0)
	{
		AIM_ASSERT(FdxCmdTxStatus(ul_Handle2, &x_TxStatus));

		if (ul_PortConfig == FDX_REDUNDANT)
			printf("\r\n Mode: %d Status: %d TxFrames PortA: %d PortB: %d \n", x_TxStatus.ul_TransmitMode, (int)x_TxStatus.e_Status, x_TxStatus.ul_Frames, x_TxStatus.ul_FramesPortB);
		else
			printf("\r\n Mode: %d Status: %d TxFrames %d\n", x_TxStatus.ul_TransmitMode, (int)x_TxStatus.e_Status, x_TxStatus.ul_Frames);
	}

	x_TxControl.ul_Count = ul_Cycles;               /* 0 = cyclic transmission */
	x_TxControl.e_StartMode = FDX_START;
	x_TxControl.e_ExtendedStopMode = FDX_ESTOP_NOT_USED;



	if (ul_Handle2 != 0)
	{
		AIM_ASSERT(FdxCmdTxControl(ul_Handle1, &x_TxControl));
		AIM_ASSERT(FdxCmdTxControl(ul_Handle2, &x_TxControl));
	}
	else
	{
		AIM_ASSERT(FdxCmdTxControl(ul_Handle1, &x_TxControl));
	}

	if (ul_Handle2 == 0)
	{
		AIM_ASSERT(FdxCmdTxStatus(ul_Handle1, &x_TxStatus));

		if (ul_PortConfig == FDX_REDUNDANT)
			printf("\r\n Mode: %d Status: %d TxFrames PortA: %d PortB: %d \n", x_TxStatus.ul_TransmitMode, (int)x_TxStatus.e_Status, x_TxStatus.ul_Frames, x_TxStatus.ul_FramesPortB);
		else
			printf("\r\n Mode: %d Status: %d TxFrames %d\n", x_TxStatus.ul_TransmitMode, (int)x_TxStatus.e_Status, x_TxStatus.ul_Frames);
	}
	else
	{
		AIM_ASSERT(FdxCmdTxStatus(ul_Handle1, &x_TxStatus));

		if (ul_PortConfig == FDX_REDUNDANT)
			printf("\r\n Mode: %d Status: %d TxFrames PortA: %d PortB: %d \n", x_TxStatus.ul_TransmitMode, (int)x_TxStatus.e_Status, x_TxStatus.ul_Frames, x_TxStatus.ul_FramesPortB);
		else
			printf("\r\n Mode: %d Status: %d TxFrames %d\n", x_TxStatus.ul_TransmitMode, (int)x_TxStatus.e_Status, x_TxStatus.ul_Frames);

		AIM_ASSERT(FdxCmdTxStatus(ul_Handle2, &x_TxStatus));

		if (ul_PortConfig == FDX_REDUNDANT)
			printf("\r\n Mode: %d Status: %d TxFrames PortA: %d PortB: %d \n", x_TxStatus.ul_TransmitMode, (int)x_TxStatus.e_Status, x_TxStatus.ul_Frames, x_TxStatus.ul_FramesPortB);
		else
			printf("\r\n Mode: %d Status: %d TxFrames %d\n", x_TxStatus.ul_TransmitMode, (int)x_TxStatus.e_Status, x_TxStatus.ul_Frames);
	}

	/*
	AIM_WAIT(500);
	}
	*/

	printf("\r\n Stop TX and reading TxStatus");

	x_TxControl.e_StartMode = FDX_STOP;
	AIM_ASSERT(FdxCmdTxControl(ul_Handle1, &x_TxControl));

	printf("\r\n ...");

	if (ul_Handle2 != 0)
	{
		AIM_ASSERT(FdxCmdTxControl(ul_Handle2, &x_TxControl));
		printf(" ...");
	}


	if (ul_Handle2 == 0)
	{
		AIM_ASSERT(FdxCmdTxStatus(ul_Handle1, &x_TxStatus));

		if (ul_PortConfig == FDX_REDUNDANT)
			printf("\r\n Mode: %d Status: %d TxFrames PortA: %d PortB: %d \n", x_TxStatus.ul_TransmitMode, (int)x_TxStatus.e_Status, x_TxStatus.ul_Frames, x_TxStatus.ul_FramesPortB);
		else
			printf("\r\n Mode: %d Status: %d TxFrames %d\n", x_TxStatus.ul_TransmitMode, (int)x_TxStatus.e_Status, x_TxStatus.ul_Frames);
	}
	else
	{
		AIM_ASSERT(FdxCmdTxStatus(ul_Handle1, &x_TxStatus));

		if (ul_PortConfig == FDX_REDUNDANT)
			printf("\r\n Mode: %d Status: %d TxFrames PortA: %d PortB: %d \n", x_TxStatus.ul_TransmitMode, (int)x_TxStatus.e_Status, x_TxStatus.ul_Frames, x_TxStatus.ul_FramesPortB);
		else
			printf("\r\n Mode: %d Status: %d TxFrames %d\n", x_TxStatus.ul_TransmitMode, (int)x_TxStatus.e_Status, x_TxStatus.ul_Frames);

		AIM_ASSERT(FdxCmdTxStatus(ul_Handle2, &x_TxStatus));

		if (ul_PortConfig == FDX_REDUNDANT)
			printf("\r\n Mode: %d Status: %d TxFrames PortA: %d PortB: %d \n", x_TxStatus.ul_TransmitMode, (int)x_TxStatus.e_Status, x_TxStatus.ul_Frames, x_TxStatus.ul_FramesPortB);
		else

			printf("\r\n Mode: %d Status: %d TxFrames %d\n", x_TxStatus.ul_TransmitMode, (int)x_TxStatus.e_Status, x_TxStatus.ul_Frames);
	}

	return(FDX_OK);
}

/****************************************************************************
Function    : l_SetTxQueueFragmented
-----------------------------------------------------------------------------
Create      :
-----------------------------------------------------------------------------

Inputs      : -

Outputs     : -

Description : .Testfunction for Init TX Port and create TX Queue

*****************************************************************************/
AiInt32 l_SetTxQueueFragmented(AiUInt32 ul_Handle)
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

	AiUInt8 Dt[200];

	printf("\n FdxCmdTxPortInit...");

	x_PortInitIn.ul_PortMap = 1;
	AIM_ASSERT(FdxCmdTxPortInit(ul_Handle, &x_PortInitIn, &x_PortInitOut));

	printf("\n FdxCmdTxModeControl...");
	x_TxModeControl.ul_TransmitMode = FDX_TX_GENERIC;
	AIM_ASSERT(FdxCmdTxModeControl(ul_Handle, &x_TxModeControl));

	printf("\n FdxCmdTxQueueCreate...");
	x_TxQueueCreate.ul_QueueSize = 0;
	AIM_ASSERT(FdxCmdTxQueueCreate(ul_Handle, &x_TxQueueCreate, &x_TxQueueInfo));

	memset(&My_Frame.x_Frame, 0, sizeof(TY_FDX_TX_FRAME_HEADER));

	My_Frame.x_Frame.uc_FrameType = FDX_TX_FRAME_STD;
	My_Frame.x_Frame.x_FrameAttrib.uc_NetSelect = FDX_TX_FRAME_BOTH;
	My_Frame.x_Frame.x_FrameAttrib.uc_ExternalStrobe = FDX_DIS;
	My_Frame.x_Frame.x_FrameAttrib.uc_FrameStartMode = FDX_TX_FRAME_START_PGWT;
	My_Frame.x_Frame.x_FrameAttrib.uc_PayloadBufferMode = FDX_TX_FRAME_PBM_STD;
	My_Frame.x_Frame.x_FrameAttrib.uc_PayloadGenerationMode = FDX_TX_FRAME_PGM_USER;
	My_Frame.x_Frame.x_FrameAttrib.uc_PreambleCount = FDX_TX_FRAME_PRE_DEF;
	My_Frame.x_Frame.x_FrameAttrib.ul_BufferQueueHandle = 0;                   /* MOD was 0 */
	My_Frame.x_Frame.x_FrameAttrib.ul_InterFrameGap = 25;                      /* MOD: 25=1usec; was 1000=40usec; */
	My_Frame.x_Frame.x_FrameAttrib.ul_PacketGroupWaitTime = 1000;              /* MOD: 1000=1msec; was 0=0usec; */

	My_Frame.x_Frame.x_FrameAttrib.ul_PhysErrorInjection = FDX_TX_FRAME_ERR_OFF;
	My_Frame.x_Frame.x_FrameAttrib.ul_Skew = 0;
	My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize = 64;                          /* MOD: was 1000; */
	My_Frame.x_Frame.x_FrameAttrib.uw_SequenceNumberInit = FDX_TX_FRAME_SEQ_INIT_AUTO;
	My_Frame.x_Frame.x_FrameAttrib.uw_SequenceNumberOffset = FDX_TX_FRAME_SEQ_OFFS_AUTO;
	My_Frame.x_Frame.x_FrameAttrib.uc_TxIntEnable = FDX_DIS;
	My_Frame.x_Frame.x_FrameAttrib.ul_IntIdent = 0;

	for (i = 0; i < 1000; i++)
		My_Frame.uc_Data[i] = (unsigned char)i;

	/* MAC */
	Dt[0] = 0x03; Dt[1] = 0x00; Dt[2] = 0x00; Dt[3] = 0x00; Dt[4] = 0x00; Dt[5] = 0x0a;
	Dt[6] = 0x02; Dt[7] = 0x00; Dt[8] = 0x00; Dt[9] = 0x01; Dt[10] = 0x21; Dt[11] = 0x20;
	Dt[12] = 0x08; Dt[13] = 0x00;
	/* IP */
	Dt[14] = 0x45; Dt[15] = 0x00;
	Dt[16] = 0x00; Dt[17] = 0x2c;
	Dt[18] = 0x00; Dt[19] = 0x05;
	Dt[20] = 0x20; Dt[21] = 0x00;
	Dt[22] = 0x01; Dt[23] = 0x11;
	Dt[24] = 0x00; Dt[25] = 0x00;
	Dt[26] = 0x0a; Dt[27] = 0x01; Dt[28] = 0x21; Dt[29] = 0x01;
	Dt[30] = 0xe0; Dt[31] = 0xe0; Dt[32] = 0x00; Dt[33] = 0x0a;
	*((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
	/* UDP */
	Dt[34] = 0x00; Dt[35] = 0x01; Dt[36] = 0x00; Dt[37] = 0x02;   Dt[38] = 0x00; Dt[39] = 0x18; Dt[40] = 0x00; Dt[41] = 0x00;
	/* Data */
	Dt[42] = 0x00; Dt[43] = 0x01; Dt[44] = 0x02; Dt[45] = 0x03;   Dt[46] = 0x04; Dt[47] = 0x05; Dt[48] = 0x06; Dt[49] = 0x07;
	Dt[50] = 0x08; Dt[51] = 0x09; Dt[52] = 0x0A; Dt[53] = 0x0B;   Dt[54] = 0x0C; Dt[55] = 0x0D; Dt[56] = 0x0E; Dt[57] = 0x0F;
	Dt[58] = 0x30;
	for (i = 0; i <= 58; i++)
		My_Frame.uc_Data[i] = (unsigned char)Dt[i];

	printf("\n FdxCmdTxQueueWrite Frame 1 ...");
	AIM_ASSERT(FdxCmdTxQueueWrite(ul_Handle, FDX_TX_FRAME_HEADER_GENERIC, 1, sizeof(My_Frame), &My_Frame));

	for (i = 0; i < 1000; i++)

		My_Frame.uc_Data[i] = 2;

	My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize = 103;
	/* MAC */
	Dt[0] = 0x03; Dt[1] = 0x00; Dt[2] = 0x00; Dt[3] = 0x00; Dt[4] = 0x00; Dt[5] = 0x0a;
	Dt[6] = 0x02; Dt[7] = 0x00; Dt[8] = 0x00; Dt[9] = 0x01; Dt[10] = 0x21; Dt[11] = 0x20;
	Dt[12] = 0x08; Dt[13] = 0x00;
	/* Ip */
	Dt[14] = 0x45; Dt[15] = 0x00;
	Dt[16] = 0x00; Dt[17] = 0x54;
	Dt[18] = 0x00; Dt[19] = 0x05;
	Dt[20] = 0x20; Dt[21] = 0x03;
	Dt[22] = 0x01; Dt[23] = 0x11;
	Dt[24] = 0x00; Dt[25] = 0x00;
	Dt[26] = 0x0a; Dt[27] = 0x01; Dt[28] = 0x21; Dt[29] = 0x01;
	Dt[30] = 0xe0; Dt[31] = 0xe0; Dt[32] = 0x00; Dt[33] = 0x0a;
	*((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
	/* UDP */
	Dt[34] = 0x00; Dt[35] = 0x01; Dt[36] = 0x00; Dt[37] = 0x02;   Dt[38] = 0x00; Dt[39] = 0x38; Dt[40] = 0x00; Dt[41] = 0x00;
	/* Data */
	Dt[42] = 0x10; Dt[43] = 0x11; Dt[44] = 0x12; Dt[45] = 0x13;   Dt[46] = 0x14; Dt[47] = 0x15; Dt[48] = 0x16; Dt[49] = 0x17;
	Dt[50] = 0x18; Dt[51] = 0x19; Dt[52] = 0x1a; Dt[53] = 0x1b;   Dt[54] = 0x1c; Dt[55] = 0x1d; Dt[56] = 0x1e; Dt[57] = 0x1f;
	Dt[58] = 0x20; Dt[59] = 0x21; Dt[60] = 0x22; Dt[61] = 0x23;   Dt[62] = 0x24; Dt[63] = 0x25; Dt[64] = 0x26; Dt[65] = 0x27;
	Dt[66] = 0x28; Dt[67] = 0x29; Dt[68] = 0x2a; Dt[69] = 0x2b;   Dt[70] = 0x2c; Dt[71] = 0x2d; Dt[72] = 0x2e; Dt[73] = 0x2f;
	Dt[74] = 0x30; Dt[75] = 0x31; Dt[76] = 0x32; Dt[77] = 0x33;   Dt[78] = 0x34; Dt[79] = 0x35; Dt[80] = 0x36; Dt[81] = 0x37;
	Dt[82] = 0x38; Dt[83] = 0x39; Dt[84] = 0x3a; Dt[85] = 0x3b;   Dt[86] = 0x3c; Dt[87] = 0x3d; Dt[88] = 0x3e; Dt[89] = 0x3f;
	Dt[90] = 0xf1; Dt[91] = 0xf2; Dt[92] = 0xf3; Dt[93] = 0xf4;   Dt[94] = 0xf5; Dt[95] = 0xf6; Dt[96] = 0xf7; Dt[97] = 0xf8;
	for (i = 0; i <= 97; i++)
		My_Frame.uc_Data[i] = (unsigned char)Dt[i];

	printf("\n FdxCmdTxQueueWrite Frame 2 ...");
	AIM_ASSERT(FdxCmdTxQueueWrite(ul_Handle, FDX_TX_FRAME_HEADER_GENERIC, 1, sizeof(My_Frame), &My_Frame));

	for (i = 0; i < 1000; i++)
		My_Frame.uc_Data[i] = 3;

	My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize = 127;
	/* MAC */
	Dt[0] = 0x03; Dt[1] = 0x00; Dt[2] = 0x00; Dt[3] = 0x00; Dt[4] = 0x00; Dt[5] = 0x0a;
	Dt[6] = 0x02; Dt[7] = 0x00; Dt[8] = 0x00; Dt[9] = 0x01; Dt[10] = 0x21; Dt[11] = 0x20;
	Dt[12] = 0x08; Dt[13] = 0x00;
	/* IP */
	Dt[14] = 0x45; Dt[15] = 0x00;
	Dt[16] = 0x00; Dt[17] = 0x6c;
	Dt[18] = 0x00; Dt[19] = 0x05;
	Dt[20] = 0x20; Dt[21] = 0x0b;
	Dt[22] = 0x01; Dt[23] = 0x11;
	Dt[24] = 0x00; Dt[25] = 0x00;
	Dt[26] = 0x0a; Dt[27] = 0x01; Dt[28] = 0x21; Dt[29] = 0x01;
	Dt[30] = 0xe0; Dt[31] = 0xe0; Dt[32] = 0x00; Dt[33] = 0x0a;
	*((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
	/* UDP */
	Dt[34] = 0x00; Dt[35] = 0x01; Dt[36] = 0x00; Dt[37] = 0x02;  Dt[38] = 0x00; Dt[39] = 0x58; Dt[40] = 0x00; Dt[41] = 0x00;
	/* Data */
	Dt[42] = 0x40; Dt[43] = 0x41; Dt[44] = 0x42; Dt[45] = 0x43;  Dt[46] = 0x44; Dt[47] = 0x45; Dt[48] = 0x46; Dt[49] = 0x47;
	Dt[50] = 0x48; Dt[51] = 0x49; Dt[52] = 0x4a; Dt[53] = 0x4b;  Dt[54] = 0x4c; Dt[55] = 0x4d; Dt[56] = 0x4e; Dt[57] = 0x4f;
	Dt[58] = 0x50; Dt[59] = 0x51; Dt[60] = 0x52; Dt[61] = 0x53;  Dt[62] = 0x54; Dt[63] = 0x55; Dt[64] = 0x56; Dt[65] = 0x57;
	Dt[66] = 0x58; Dt[67] = 0x59; Dt[68] = 0x5a; Dt[69] = 0x5b;  Dt[70] = 0x5c; Dt[71] = 0x5d; Dt[72] = 0x5e; Dt[73] = 0x5f;
	Dt[74] = 0x60; Dt[75] = 0x61; Dt[76] = 0x62; Dt[77] = 0x63;  Dt[78] = 0x64; Dt[79] = 0x65; Dt[80] = 0x66; Dt[81] = 0x67;
	Dt[82] = 0x68; Dt[83] = 0x69; Dt[84] = 0x6a; Dt[85] = 0x6b;  Dt[86] = 0x6c; Dt[87] = 0x6d; Dt[88] = 0x6e; Dt[89] = 0x6f;
	Dt[90] = 0x70; Dt[91] = 0x71; Dt[92] = 0x72; Dt[93] = 0x73;  Dt[94] = 0x74; Dt[95] = 0x75; Dt[96] = 0x76; Dt[97] = 0x77;
	Dt[98] = 0x78; Dt[99] = 0x79; Dt[100] = 0x7a; Dt[101] = 0x7b;  Dt[102] = 0x7c; Dt[103] = 0x7d; Dt[104] = 0x7e; Dt[105] = 0x7f;
	Dt[106] = 0x80; Dt[107] = 0x81; Dt[108] = 0x82; Dt[109] = 0x83;  Dt[110] = 0x84; Dt[111] = 0x85; Dt[112] = 0x86; Dt[113] = 0x87;
	Dt[114] = 0x88; Dt[115] = 0x89; Dt[116] = 0x8a; Dt[117] = 0x8b;  Dt[118] = 0x8c; Dt[119] = 0x8d; Dt[120] = 0x8e; Dt[121] = 0x8f;
	for (i = 0; i <= 121; i++)
		My_Frame.uc_Data[i] = (unsigned char)Dt[i];

	printf("\n FdxCmdTxQueueWrite Frame 3 ...");
	AIM_ASSERT(FdxCmdTxQueueWrite(ul_Handle, FDX_TX_FRAME_HEADER_GENERIC, 1, sizeof(My_Frame), &My_Frame));

	for (i = 0; i < 1000; i++)
		My_Frame.uc_Data[i] = 4;

	My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize = 71;
	/* MAC */
	Dt[0] = 0x03; Dt[1] = 0x00; Dt[2] = 0x00; Dt[3] = 0x00; Dt[4] = 0x00; Dt[5] = 0x0a;
	Dt[6] = 0x02; Dt[7] = 0x00; Dt[8] = 0x00; Dt[9] = 0x01; Dt[10] = 0x21; Dt[11] = 0x20;
	Dt[12] = 0x08; Dt[13] = 0x00;
	/* IP */
	Dt[14] = 0x45; Dt[15] = 0x00;
	Dt[16] = 0x00; Dt[17] = 0x34;
	Dt[18] = 0x00; Dt[19] = 0x05;
	Dt[20] = 0x20; Dt[21] = 0x16;
	Dt[22] = 0x01; Dt[23] = 0x11;
	Dt[24] = 0x00; Dt[25] = 0x00;
	Dt[26] = 0x0a; Dt[27] = 0x01; Dt[28] = 0x21; Dt[29] = 0x01;
	Dt[30] = 0xe0; Dt[31] = 0xe0; Dt[32] = 0x00; Dt[33] = 0x0a;
	*((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
	/* UDP */
	Dt[34] = 0x00; Dt[35] = 0x01; Dt[36] = 0x00; Dt[37] = 0x02;   Dt[38] = 0x00; Dt[39] = 0x20; Dt[40] = 0x00; Dt[41] = 0x00;
	/* Data */
	Dt[42] = 0x90; Dt[43] = 0x91; Dt[44] = 0x92; Dt[45] = 0x93;   Dt[46] = 0x94; Dt[47] = 0x95; Dt[48] = 0x96; Dt[49] = 0x97;
	Dt[50] = 0x98; Dt[51] = 0x99; Dt[52] = 0x9a; Dt[53] = 0x9b;   Dt[54] = 0x9c; Dt[55] = 0x9d; Dt[56] = 0x9e; Dt[57] = 0x9f;
	Dt[58] = 0xa0; Dt[59] = 0xa1; Dt[60] = 0xa2; Dt[61] = 0xa3;   Dt[62] = 0xa4; Dt[63] = 0xa5; Dt[64] = 0xa6; Dt[65] = 0xa7;
	for (i = 0; i <= 65; i++)
		My_Frame.uc_Data[i] = (unsigned char)Dt[i];

	printf("\n FdxCmdTxQueueWrite Frame 4 ...");
	AIM_ASSERT(FdxCmdTxQueueWrite(ul_Handle, FDX_TX_FRAME_HEADER_GENERIC, 1, sizeof(My_Frame), &My_Frame));


	for (i = 0; i < 1000; i++)
		My_Frame.uc_Data[i] = 5;

	My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize = 71;
	/* MAC */
	Dt[0] = 0x03; Dt[1] = 0x00; Dt[2] = 0x00; Dt[3] = 0x00; Dt[4] = 0x00; Dt[5] = 0x0a;
	Dt[6] = 0x02; Dt[7] = 0x00; Dt[8] = 0x00; Dt[9] = 0x01; Dt[10] = 0x21; Dt[11] = 0x20;
	/* IP */
	Dt[12] = 0x08; Dt[13] = 0x00;
	Dt[14] = 0x45; Dt[15] = 0x00;
	Dt[16] = 0x00; Dt[17] = 0x34;
	Dt[18] = 0x00; Dt[19] = 0x05;
	Dt[20] = 0x20; Dt[21] = 0x1a;
	Dt[22] = 0x01; Dt[23] = 0x11;
	Dt[24] = 0x00; Dt[25] = 0x00;
	Dt[26] = 0x0a; Dt[27] = 0x01; Dt[28] = 0x21; Dt[29] = 0x01;
	Dt[30] = 0xe0; Dt[31] = 0xe0; Dt[32] = 0x00; Dt[33] = 0x0a;
	*((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
	/* UDP */
	Dt[34] = 0x00; Dt[35] = 0x01; Dt[36] = 0x00; Dt[37] = 0x02;   Dt[38] = 0x00; Dt[39] = 0x20; Dt[40] = 0x00; Dt[41] = 0x00;
	/* Data */
	Dt[42] = 0xa8; Dt[43] = 0xa9; Dt[44] = 0xaa; Dt[45] = 0xab;   Dt[46] = 0xac; Dt[47] = 0xad; Dt[48] = 0xae; Dt[49] = 0xaf;
	Dt[50] = 0xb0; Dt[51] = 0xb1; Dt[52] = 0xb2; Dt[53] = 0xb3;   Dt[54] = 0xb4; Dt[55] = 0xb5; Dt[56] = 0xb6; Dt[57] = 0xb7;
	Dt[58] = 0xb8; Dt[59] = 0xb9; Dt[60] = 0xba; Dt[61] = 0xbb;   Dt[62] = 0xbc; Dt[63] = 0xbd;
	Dt[64] = 0xbe; Dt[65] = 0xbf;
	for (i = 0; i <= 65; i++)
		My_Frame.uc_Data[i] = (unsigned char)Dt[i];

	printf("\n FdxCmdTxQueueWrite Frame 5 ...");
	AIM_ASSERT(FdxCmdTxQueueWrite(ul_Handle, FDX_TX_FRAME_HEADER_GENERIC, 1, sizeof(My_Frame), &My_Frame));

	for (i = 0; i < 1000; i++)
		My_Frame.uc_Data[i] = 6;

	My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize = 87;
	/* MAC */
	Dt[0] = 0x03; Dt[1] = 0x00; Dt[2] = 0x00; Dt[3] = 0x00; Dt[4] = 0x00; Dt[5] = 0x0a;
	Dt[6] = 0x02; Dt[7] = 0x00; Dt[8] = 0x00; Dt[9] = 0x01; Dt[10] = 0x21; Dt[11] = 0x20;
	Dt[12] = 0x08; Dt[13] = 0x00;
	/* IP */
	Dt[14] = 0x45; Dt[15] = 0x00;
	Dt[16] = 0x00; Dt[17] = 0x44;
	Dt[18] = 0x00; Dt[19] = 0x05;
	Dt[20] = 0x20; Dt[21] = 0x1e;
	Dt[22] = 0x01; Dt[23] = 0x11;
	Dt[24] = 0x00; Dt[25] = 0x00;
	Dt[26] = 0x0a; Dt[27] = 0x01; Dt[28] = 0x21; Dt[29] = 0x01;
	Dt[30] = 0xe0; Dt[31] = 0xe0; Dt[32] = 0x00; Dt[33] = 0x0a;
	*((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
	/* UDP */
	Dt[34] = 0x00; Dt[35] = 0x01; Dt[36] = 0x00; Dt[37] = 0x02;   Dt[38] = 0x00; Dt[39] = 0x28; Dt[40] = 0x00; Dt[41] = 0x00;
	/* Data */
	Dt[42] = 0xc0; Dt[43] = 0xc1; Dt[44] = 0xc2; Dt[45] = 0xc3;   Dt[46] = 0xc4; Dt[47] = 0xc5; Dt[48] = 0xc6; Dt[49] = 0xc7;
	Dt[50] = 0xc8; Dt[51] = 0xc9; Dt[52] = 0xca; Dt[53] = 0xcb;   Dt[54] = 0xcc; Dt[55] = 0xcd; Dt[56] = 0xce; Dt[57] = 0xcf;
	Dt[58] = 0xd0; Dt[59] = 0xd1; Dt[60] = 0xd2; Dt[61] = 0xd3;   Dt[62] = 0xd4; Dt[63] = 0xd5; Dt[64] = 0xd6; Dt[65] = 0xd7;
	Dt[66] = 0xd8; Dt[67] = 0xd9; Dt[68] = 0xda; Dt[69] = 0xdb;   Dt[70] = 0xdc; Dt[71] = 0xdd; Dt[72] = 0xde; Dt[73] = 0xdf;
	Dt[74] = 0x55; Dt[75] = 0x55; Dt[76] = 0x55; Dt[77] = 0x55;   Dt[78] = 0x55; Dt[79] = 0x55; Dt[80] = 0x55; Dt[81] = 0x55;
	for (i = 0; i <= 81; i++)
		My_Frame.uc_Data[i] = (unsigned char)Dt[i];

	printf("\n FdxCmdTxQueueWrite Frame 6 ...");
	AIM_ASSERT(FdxCmdTxQueueWrite(ul_Handle, FDX_TX_FRAME_HEADER_GENERIC, 1, sizeof(My_Frame), &My_Frame));

	for (i = 0; i < 1000; i++)
		My_Frame.uc_Data[i] = 7;

	My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize = 64;
	/* MAC */
	Dt[0] = 0x03; Dt[1] = 0x00; Dt[2] = 0x00; Dt[3] = 0x00; Dt[4] = 0x00; Dt[5] = 0x0a;
	Dt[6] = 0x02; Dt[7] = 0x00; Dt[8] = 0x00; Dt[9] = 0x01; Dt[10] = 0x21; Dt[11] = 0x20;
	Dt[12] = 0x08; Dt[13] = 0x00;
	/* Ip */
	Dt[14] = 0x45; Dt[15] = 0x00;
	Dt[16] = 0x00; Dt[17] = 0x2c;
	Dt[18] = 0x00; Dt[19] = 0x05;
	Dt[20] = 0x00; Dt[21] = 0x24;
	Dt[22] = 0x01; Dt[23] = 0x11;
	Dt[24] = 0x00; Dt[25] = 0x00;
	Dt[26] = 0x0a; Dt[27] = 0x01; Dt[28] = 0x21; Dt[29] = 0x01;
	Dt[30] = 0xe0; Dt[31] = 0xe0; Dt[32] = 0x00; Dt[33] = 0x0a;
	*((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
	/* UDP */
	Dt[34] = 0x00; Dt[35] = 0x01; Dt[36] = 0x00; Dt[37] = 0x02;   Dt[38] = 0x00; Dt[39] = 0x18; Dt[40] = 0x00; Dt[41] = 0x00;
	/* Data */
	Dt[42] = 0xe0; Dt[43] = 0xe1; Dt[44] = 0xe2; Dt[45] = 0xe3;   Dt[46] = 0xe4; Dt[47] = 0xe5; Dt[48] = 0xe6; Dt[49] = 0xe7;
	Dt[50] = 0xe8; Dt[51] = 0xe9; Dt[52] = 0xea; Dt[53] = 0xeb;   Dt[54] = 0xec; Dt[55] = 0xed; Dt[56] = 0xee; Dt[57] = 0xef;
	Dt[58] = 0xef;
	for (i = 0; i <= 58; i++)
		My_Frame.uc_Data[i] = (unsigned char)Dt[i];

	printf("\n FdxCmdTxQueueWrite Frame 7 ...");
	AIM_ASSERT(FdxCmdTxQueueWrite(ul_Handle, FDX_TX_FRAME_HEADER_GENERIC, 1, sizeof(My_Frame), &My_Frame));


	return(FDX_OK);
}



/****************************************************************************
Function    : l_TestTxSimulateVl
-----------------------------------------------------------------------------
Create      :
-----------------------------------------------------------------------------

Inputs      : -

Outputs     : -

Description : Testfunction for Simulation Transmitter Functions writing
direct to VL Buffer

*****************************************************************************/
AiInt32 l_TestTxSimulateVl(AiUInt32 ul_Handle)
{
	TY_FDX_TX_VL_WRITE_IN x_TxVLWriteIn;
	TY_FDX_TX_VL_WRITE_OUT x_TxVLWriteOut;
	TY_FDX_PORT_INIT_IN x_PortInitIn;
	TY_FDX_PORT_INIT_OUT x_PortInitOut;
	TY_FDX_TX_MODE_CTRL x_TxModeControl;
	TY_FDX_TRANSMIT_VL x_TxCreateVL;
	/*AiUInt32 ul_HandleCnt = 0; */
	AiUInt8  Dt[1600];
	TY_FDX_TX_CTRL x_TxControl;
	AiInt16 finish = 0;

	AiInt16 ch;

	printf("\n FdxCmdTxPortInit...");

	x_PortInitIn.ul_PortMap = 1;
	AIM_ASSERT(FdxCmdTxPortInit(ul_Handle, &x_PortInitIn, &x_PortInitOut));

	printf("\n FdxCmdTxModeControl...");
	x_TxModeControl.ul_TransmitMode = FDX_TX_INDIVIDUAL;
	AIM_ASSERT(FdxCmdTxModeControl(ul_Handle, &x_TxModeControl));



	/* VL 33 ********************************************************************************** */
	printf("\n FdxCmdTxCreateVL...");
	x_TxCreateVL.ul_VlId = 11;           /* VL */
	x_TxCreateVL.ul_SubVls = 4;            /* Number of Sub VLs */
	x_TxCreateVL.ul_Bag = 1;            /* ms */
	x_TxCreateVL.ul_MaxFrameLength = 1000;         /* bytes */
	x_TxCreateVL.ul_FrameBufferSize = 0;            /* Default Frame Buffer Size */
	x_TxCreateVL.ul_MACSourceLSLW = 0x00089aC0;   /* */
	x_TxCreateVL.ul_MACSourceMSLW = 0x00000200;   /* */
	x_TxCreateVL.ul_NetSelect = FDX_TX_FRAME_BOTH;
	x_TxCreateVL.ul_Skew = 0;
	AIM_ASSERT(FdxCmdTxCreateVL(ul_Handle, &x_TxCreateVL));
	/* End VL 66 ********************************************************************************** */


	/* VL 66 ********************************************************************************** */
	printf("\n FdxCmdTxCreateVL...");
	x_TxCreateVL.ul_VlId = 22;           /* VL */
	x_TxCreateVL.ul_SubVls = 3;            /* Number of Sub VLs */
	x_TxCreateVL.ul_Bag = 32;           /* ms */
	x_TxCreateVL.ul_MaxFrameLength = 500;          /* bytes */
	x_TxCreateVL.ul_FrameBufferSize = 0;            /* Default Frame Buffer Size */
	x_TxCreateVL.ul_MACSourceLSLW = 0x000bcde0;   /* */
	x_TxCreateVL.ul_MACSourceMSLW = 0x00000200;   /* */
	x_TxCreateVL.ul_NetSelect = FDX_TX_FRAME_BOTH;
	x_TxCreateVL.ul_Skew = 0;
	AIM_ASSERT(FdxCmdTxCreateVL(ul_Handle, &x_TxCreateVL));
	/* End VL 66 ********************************************************************************** */


	/* VL 99 ********************************************************************************** */
	printf("\n FdxCmdTxCreateVL...");
	x_TxCreateVL.ul_VlId = 33;           /* VL */
	x_TxCreateVL.ul_SubVls = 2;            /* Number of Sub VLs */
	x_TxCreateVL.ul_Bag = 64;           /* ms */
	x_TxCreateVL.ul_MaxFrameLength = 500;          /* bytes */
	x_TxCreateVL.ul_FrameBufferSize = 0;            /* Default Frame Buffer Size */
	x_TxCreateVL.ul_MACSourceLSLW = 0x00012340;   /* */
	x_TxCreateVL.ul_MACSourceMSLW = 0x00000200;   /* */
	x_TxCreateVL.ul_NetSelect = FDX_TX_FRAME_BOTH;
	x_TxCreateVL.ul_Skew = 0;
	AIM_ASSERT(FdxCmdTxCreateVL(ul_Handle, &x_TxCreateVL));
	/* End VL 99 ********************************************************************************** */

	/* VL 77 ********************************************************************************** */
	printf("\n FdxCmdTxCreateVL...");
	x_TxCreateVL.ul_VlId = 44;           /* VL */
	x_TxCreateVL.ul_SubVls = 1;            /* Number of Sub VLs */
	x_TxCreateVL.ul_Bag = 128;          /* ms */
	x_TxCreateVL.ul_MaxFrameLength = 1500;         /* bytes */
	x_TxCreateVL.ul_FrameBufferSize = 0;            /* Default Frame Buffer Size */
	x_TxCreateVL.ul_MACSourceLSLW = 0x000123E0;   /* */
	x_TxCreateVL.ul_MACSourceMSLW = 0x00000200;   /* */
	x_TxCreateVL.ul_NetSelect = FDX_TX_FRAME_BOTH;
	x_TxCreateVL.ul_Skew = 0;
	AIM_ASSERT(FdxCmdTxCreateVL(ul_Handle, &x_TxCreateVL));
	/* End VL 77 ********************************************************************************** */


	printf("\n FdxCmdTxControl (Start)...");
	x_TxControl.e_StartMode = FDX_START;
	x_TxControl.ul_Count = 0;

	AIM_ASSERT(FdxCmdTxControl(ul_Handle, &x_TxControl));

	printf("\n Press key to read status. ('e' to abort Read Status)");

	do

	{
		ch = GetConsoleCharacter();
		if (ch == 'e')
			finish = 1;

		switch (ch)
		{
		case '1':
			x_TxVLWriteIn.ul_VlId = 11;
			x_TxVLWriteIn.ul_SubVlId = 1;
			x_TxVLWriteIn.ul_ByteCount = 103;
			x_TxVLWriteIn.pv_Data = Dt;

			/* MAC */
			Dt[0] = 0x03; Dt[1] = 0x00; Dt[2] = 0x00; Dt[3] = 0x00; Dt[4] = 0x00; Dt[5] = 0x0b;
			Dt[6] = 0x02; Dt[7] = 0x00; Dt[8] = 0x00; Dt[9] = 0x01; Dt[10] = 0x21; Dt[11] = 0x20;
			Dt[12] = 0x08; Dt[13] = 0x00;
			/* IP */
			Dt[14] = 0x45; Dt[15] = 0x00;
			Dt[16] = 0x00; Dt[17] = 0x54;
			Dt[18] = 0x00; Dt[19] = 0x01;
			Dt[20] = 0x20; Dt[21] = 0x03;
			Dt[22] = 0x01; Dt[23] = 0x11;
			Dt[24] = 0x8d; Dt[25] = 0x29;
			Dt[26] = 0x0a; Dt[27] = 0x01; Dt[28] = 0x21; Dt[29] = 0x01;
			Dt[30] = 0xe0; Dt[31] = 0xe0; Dt[32] = 0x00; Dt[33] = 0x0a;
			Dt[24] = 0x00; Dt[25] = 0x00; /* Byte 24 and 25 = Ip Header Check sum set to 0 for correct calculation*/
			*((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
			/* UDP */
			Dt[34] = 0x00; Dt[35] = 0x01; Dt[36] = 0x00; Dt[37] = 0x02; Dt[38] = 0x00; Dt[39] = 0x38; Dt[40] = 0x00; Dt[41] = 0x00;
			/* Data */
			Dt[42] = 0x10; Dt[43] = 0x11; Dt[44] = 0x12; Dt[45] = 0x13; Dt[46] = 0x14; Dt[47] = 0x15; Dt[48] = 0x16; Dt[49] = 0x17;
			Dt[50] = 0x18; Dt[51] = 0x19; Dt[52] = 0x1a; Dt[53] = 0x1b; Dt[54] = 0x1c; Dt[55] = 0x1d; Dt[56] = 0x1e; Dt[57] = 0x1f;
			Dt[58] = 0x20; Dt[59] = 0x21; Dt[60] = 0x22; Dt[61] = 0x23; Dt[62] = 0x24; Dt[63] = 0x25; Dt[64] = 0x26; Dt[65] = 0x27;
			Dt[66] = 0x28; Dt[67] = 0x29; Dt[68] = 0x2a; Dt[69] = 0x2b; Dt[70] = 0x2c; Dt[71] = 0x2d; Dt[72] = 0x2e; Dt[73] = 0x2f;
			Dt[74] = 0x30; Dt[75] = 0x31; Dt[76] = 0x32; Dt[77] = 0x33; Dt[78] = 0x34; Dt[79] = 0x35; Dt[80] = 0x36; Dt[81] = 0x37;
			Dt[82] = 0x38; Dt[83] = 0x39; Dt[84] = 0x3a; Dt[85] = 0x3b; Dt[86] = 0x3c; Dt[87] = 0x3d; Dt[88] = 0x3e; Dt[89] = 0x3f;
			Dt[90] = 0xf1; Dt[91] = 0xf2; Dt[92] = 0xf3; Dt[93] = 0xf4; Dt[94] = 0xf5; Dt[95] = 0xf6; Dt[96] = 0xf7; Dt[97] = 0xf8;

			AIM_ASSERT(FdxCmdTxVLWrite(ul_Handle, &x_TxVLWriteIn, &x_TxVLWriteOut));
			x_TxVLWriteIn.ul_SubVlId = 2;
			AIM_ASSERT(FdxCmdTxVLWrite(ul_Handle, &x_TxVLWriteIn, &x_TxVLWriteOut));
			x_TxVLWriteIn.ul_SubVlId = 3;
			AIM_ASSERT(FdxCmdTxVLWrite(ul_Handle, &x_TxVLWriteIn, &x_TxVLWriteOut));
			x_TxVLWriteIn.ul_SubVlId = 4;
			AIM_ASSERT(FdxCmdTxVLWrite(ul_Handle, &x_TxVLWriteIn, &x_TxVLWriteOut));
			x_TxVLWriteIn.ul_SubVlId = 1;
			AIM_ASSERT(FdxCmdTxVLWrite(ul_Handle, &x_TxVLWriteIn, &x_TxVLWriteOut));
			printf("\r\nFdxCmdTxVLWrite() %d Bytes written.", x_TxVLWriteOut.ul_BytesWritten);
			break;

		case '2':
			x_TxVLWriteIn.ul_VlId = 22;
			x_TxVLWriteIn.ul_SubVlId = 2;
			x_TxVLWriteIn.ul_ByteCount = 127;
			x_TxVLWriteIn.pv_Data = Dt;

			/* MAC */
			Dt[0] = 0x03; Dt[1] = 0x00; Dt[2] = 0x00; Dt[3] = 0x00; Dt[4] = 0x00; Dt[5] = 0x16;
			Dt[6] = 0x02; Dt[7] = 0x00; Dt[8] = 0x00; Dt[9] = 0x01; Dt[10] = 0x21; Dt[11] = 0x20;
			Dt[12] = 0x08; Dt[13] = 0x00;
			/* IP */
			Dt[14] = 0x45; Dt[15] = 0x00;
			Dt[16] = 0x00; Dt[17] = 0x6c;
			Dt[18] = 0x00; Dt[19] = 0x02;
			Dt[20] = 0x20; Dt[21] = 0x0b;
			Dt[22] = 0x01; Dt[23] = 0x11;
			Dt[24] = 0x8d; Dt[25] = 0x08;
			Dt[26] = 0x0a; Dt[27] = 0x01; Dt[28] = 0x21; Dt[29] = 0x01;
			Dt[30] = 0xe0; Dt[31] = 0xe0; Dt[32] = 0x00; Dt[33] = 0x0a;
			Dt[24] = 0x00; Dt[25] = 0x00; /* Byte 24 and 25 = Ip Header Check sum set to 0 for correct calculation*/
			*((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
			/* UDP */
			Dt[34] = 0x00; Dt[35] = 0x01; Dt[36] = 0x00; Dt[37] = 0x02; Dt[38] = 0x00; Dt[39] = 0x58; Dt[40] = 0x00; Dt[41] = 0x00;
			/* Data */
			Dt[42] = 0x40; Dt[43] = 0x41; Dt[44] = 0x42; Dt[45] = 0x43; Dt[46] = 0x44; Dt[47] = 0x45; Dt[48] = 0x46; Dt[49] = 0x47;
			Dt[50] = 0x48; Dt[51] = 0x49; Dt[52] = 0x4a; Dt[53] = 0x4b; Dt[54] = 0x4c; Dt[55] = 0x4d; Dt[56] = 0x4e; Dt[57] = 0x4f;
			Dt[58] = 0x50; Dt[59] = 0x51; Dt[60] = 0x52; Dt[61] = 0x53; Dt[62] = 0x54; Dt[63] = 0x55; Dt[64] = 0x56; Dt[65] = 0x57;
			Dt[66] = 0x58; Dt[67] = 0x59; Dt[68] = 0x5a; Dt[69] = 0x5b; Dt[70] = 0x5c; Dt[71] = 0x5d; Dt[72] = 0x5e; Dt[73] = 0x5f;
			Dt[74] = 0x60; Dt[75] = 0x61; Dt[76] = 0x62; Dt[77] = 0x63; Dt[78] = 0x64; Dt[79] = 0x65; Dt[80] = 0x66; Dt[81] = 0x67;
			Dt[82] = 0x68; Dt[83] = 0x69; Dt[84] = 0x6a; Dt[85] = 0x6b; Dt[86] = 0x6c; Dt[87] = 0x6d; Dt[88] = 0x6e; Dt[89] = 0x6f;
			Dt[90] = 0x70; Dt[91] = 0x71; Dt[92] = 0x72; Dt[93] = 0x73; Dt[94] = 0x74; Dt[95] = 0x75; Dt[96] = 0x76; Dt[97] = 0x77;
			Dt[98] = 0x78; Dt[99] = 0x79; Dt[100] = 0x7a; Dt[101] = 0x7b; Dt[102] = 0x7c; Dt[103] = 0x7d; Dt[104] = 0x7e; Dt[105] = 0x7f;
			Dt[106] = 0x80; Dt[107] = 0x81; Dt[108] = 0x82; Dt[109] = 0x83; Dt[110] = 0x84; Dt[111] = 0x85; Dt[112] = 0x86; Dt[113] = 0x87;
			Dt[114] = 0x88; Dt[115] = 0x89; Dt[116] = 0x8a; Dt[117] = 0x8b; Dt[118] = 0x8c; Dt[119] = 0x8d; Dt[120] = 0x8e; Dt[121] = 0x8f;

			AIM_ASSERT(FdxCmdTxVLWrite(ul_Handle, &x_TxVLWriteIn, &x_TxVLWriteOut));
			printf("\r\nFdxCmdTxVLWrite() %d Bytes written.", x_TxVLWriteOut.ul_BytesWritten);
			break;

		case '3':
			x_TxVLWriteIn.ul_VlId = 33;
			x_TxVLWriteIn.ul_SubVlId = 1;
			x_TxVLWriteIn.ul_ByteCount = 71;
			x_TxVLWriteIn.pv_Data = Dt;

			/* MAC */
			Dt[0] = 0x03; Dt[1] = 0x00; Dt[2] = 0x00; Dt[3] = 0x00; Dt[4] = 0x00; Dt[5] = 0x21;
			Dt[6] = 0x02; Dt[7] = 0x00; Dt[8] = 0x00; Dt[9] = 0x01; Dt[10] = 0x21; Dt[11] = 0x20;
			Dt[12] = 0x08; Dt[13] = 0x00;
			/* IP */
			Dt[14] = 0x45; Dt[15] = 0x00;
			Dt[16] = 0x00; Dt[17] = 0x34;
			Dt[18] = 0x00; Dt[19] = 0x03;
			Dt[20] = 0x20; Dt[21] = 0x16;
			Dt[22] = 0x01; Dt[23] = 0x11;
			Dt[24] = 0x8d; Dt[25] = 0x34;
			Dt[26] = 0x0a; Dt[27] = 0x01; Dt[28] = 0x21; Dt[29] = 0x01;
			Dt[30] = 0xe0; Dt[31] = 0xe0; Dt[32] = 0x00; Dt[33] = 0x0a;
			Dt[24] = 0x00; Dt[25] = 0x00; /* Byte 24 and 25 = Ip Header Check sum set to 0 for correct calculation*/
			*((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
			/* UDP */
			Dt[34] = 0x00; Dt[35] = 0x01; Dt[36] = 0x00; Dt[37] = 0x02; Dt[38] = 0x00; Dt[39] = 0x20; Dt[40] = 0x00; Dt[41] = 0x00;
			/* Data */
			Dt[42] = 0x90; Dt[43] = 0x91; Dt[44] = 0x92; Dt[45] = 0x93; Dt[46] = 0x94; Dt[47] = 0x95; Dt[48] = 0x96; Dt[49] = 0x97;
			Dt[50] = 0x98; Dt[51] = 0x99; Dt[52] = 0x9a; Dt[53] = 0x9b; Dt[54] = 0x9c; Dt[55] = 0x9d; Dt[56] = 0x9e; Dt[57] = 0x9f;
			Dt[58] = 0xa0; Dt[59] = 0xa1; Dt[60] = 0xa2; Dt[61] = 0xa3; Dt[62] = 0xa4; Dt[63] = 0xa5; Dt[64] = 0xa6; Dt[65] = 0xa7;

			AIM_ASSERT(FdxCmdTxVLWrite(ul_Handle, &x_TxVLWriteIn, &x_TxVLWriteOut));
			printf("\r\nFdxCmdTxVLWrite() %d Bytes written.", x_TxVLWriteOut.ul_BytesWritten);
			break;

		case '4':
			x_TxVLWriteIn.ul_VlId = 44;

			x_TxVLWriteIn.ul_SubVlId = 1;
			x_TxVLWriteIn.ul_ByteCount = 87;
			x_TxVLWriteIn.pv_Data = Dt;

			/* MAC */
			Dt[0] = 0x03; Dt[1] = 0x00; Dt[2] = 0x00; Dt[3] = 0x00; Dt[4] = 0x00; Dt[5] = 0x2c;
			Dt[6] = 0x02; Dt[7] = 0x00; Dt[8] = 0x00; Dt[9] = 0x01; Dt[10] = 0x21; Dt[11] = 0x20;
			Dt[12] = 0x08; Dt[13] = 0x00;
			/* IP */
			Dt[14] = 0x45; Dt[15] = 0x00;
			Dt[16] = 0x00; Dt[17] = 0x44;
			Dt[18] = 0x00; Dt[19] = 0x05;
			Dt[20] = 0x20; Dt[21] = 0x1e;
			Dt[22] = 0x01; Dt[23] = 0x11;
			Dt[24] = 0x8d; Dt[25] = 0x1a;
			Dt[26] = 0x0a; Dt[27] = 0x01; Dt[28] = 0x21; Dt[29] = 0x01;
			Dt[30] = 0xe0; Dt[31] = 0xe0; Dt[32] = 0x00; Dt[33] = 0x0a;
			Dt[24] = 0x00; Dt[25] = 0x00; /* Byte 24 and 25 = Ip Header Check sum set to 0 for correct calculation*/
			*((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
			/* UDP */
			Dt[34] = 0x00; Dt[35] = 0x01; Dt[36] = 0x00; Dt[37] = 0x02; Dt[38] = 0x00; Dt[39] = 0x28; Dt[40] = 0x00; Dt[41] = 0x00;
			/* Data */
			Dt[42] = 0xc0; Dt[43] = 0xc1; Dt[44] = 0xc2; Dt[45] = 0xc3; Dt[46] = 0xc4; Dt[47] = 0xc5; Dt[48] = 0xc6; Dt[49] = 0xc7;
			Dt[50] = 0xc8; Dt[51] = 0xc9; Dt[52] = 0xca; Dt[53] = 0xcb; Dt[54] = 0xcc; Dt[55] = 0xcd; Dt[56] = 0xce; Dt[57] = 0xcf;
			Dt[58] = 0xd0; Dt[59] = 0xd1; Dt[60] = 0xd2; Dt[61] = 0xd3; Dt[62] = 0xd4; Dt[63] = 0xd5; Dt[64] = 0xd6; Dt[65] = 0xd7;
			Dt[66] = 0xd8; Dt[67] = 0xd9; Dt[68] = 0xda; Dt[69] = 0xdb; Dt[70] = 0xdc; Dt[71] = 0xdd; Dt[72] = 0xde; Dt[73] = 0xdf;
			Dt[74] = 0x55; Dt[75] = 0x55; Dt[76] = 0x55; Dt[77] = 0x55; Dt[78] = 0x55; Dt[79] = 0x55; Dt[80] = 0x55; Dt[81] = 0x55;

			AIM_ASSERT(FdxCmdTxVLWrite(ul_Handle, &x_TxVLWriteIn, &x_TxVLWriteOut));
			printf("\r\nFdxCmdTxVLWrite() %d Bytes written.", x_TxVLWriteOut.ul_BytesWritten);
			break;

		default:
			printf("\r\nType 1, 2, 3 or 4 to send Data");

			break;
		}


		printf("\r\ne[xit]\r\n>");

	} while (finish == 0);

	printf("\n FdxCmdTxControl (Stop)...");
	x_TxControl.e_StartMode = FDX_STOP;
	AIM_ASSERT(FdxCmdTxControl(ul_Handle, &x_TxControl));

	return(FDX_OK);
}

/* EOF */
# AfdxSnmp

# Overview

This is a project. It can help you realize the function of sending snmp frames in the snmp protocol through the Arinc664 board and get a reply.

If you're never used arinc664 board before,or you're trying to figure out how to use vcpkg, check out my Getting Started section section for how to start using arinc664 board to create a project.

# Table of Contents

- [Overview](#Overview)
- [Table of Contents](#TableofContents)
- [Getting Started](#GettingStarted)
  - [Quick Start:Windows](###Quick-Start:Windows)
    - [Prerequisites](####Prerequisites)
    - [Install the board driver](####Install-the-board-driver)
    - [Build your own application](####Build-your-own-application)
- [Example](#Example)
- [Contributing](#Contributing)
- [License](#License)

# Getting Started

First , follow the quick start for windows,depending on what you're using.

### Quick Start : Windows

#### Prerequisites:

- Windows 7 or newer
- [Visual Studio](https://visualstudio.microsoft.com/zh-hans/) 2013 or newer
- Board driver

#### Install the board driver

1. First, you need to install the arinc664 driver. The driver is in the /driver folder. Open the folder and you will see a file named setup.exe, which is the driver installation package.
2. According to your situation, select the installation location, confirm that there is no problem, click next, and the driver will be installed automatically.
3. After the driver installation process is over, you will see the check box to *update the system driver* (selected by default), click Finish.
4. Automatically jump out of the *Device Driver Installation Wizard*, to continue, click next.
5. After the two installations are over, open [*C:\Program Files\AIM GmbH\Arinc 664 Windows BSP 19.2.1*](file:///C:/Program Files/AIM GmbH/Arinc 664 Windows BSP 19.2.1) (if installed by default), and you will see the driver file directory you installed.

#### Building your own application

1. Open visual studio 2013 (or the version that conforms to the [Prerequisites](####Prerequisites) you are using) and create an empty Visual C++ project.

2. **Include the *AiFdx_def.h* header file**

   Include the AiFdx_def.h header file in your application’s modules that want to make use of Arinc
   664 functionality.

   ```c
   //Include the AIM Arinc 664 API header file
   #include "AiFdx_def.h"
   int main(int argc, char *argv[]){
       return 0;
   }
   ```

3. **Adding 664 API include path**

   Add the Arinc 664 BSP’s include directory to the "Additional Include Directories" property in your
   project’s C/C++ -> General configuration section.

4. **Adding _AIM_WINDOWS preprocessor macro**

   Add the preprocessor macro _AIM_WINDOWS to the "Preprocessor Definitions" property in your
   project’s C/C++ -> Preprocessor configuration section.

5. **Adding aim_fdx.19.lib dependency**

   Now add the aim_fdx.19.lib library to the "Additional Dependecies" property in your project’s Linker
   -> Input configuration section.

6. **Adding library search path**

   As a last step, append the search path for the Arinc 664 API library to the "Additional Library
   Directories" property in your project’s Linker -> General configuration section. If building a 32Bit
   application you have to add the lib32 directory of the BSP installation directory. In case of building
   a 64Bit application, lib64 has to be specified instead.

**Now, you can start to construct your Arinc664 project, good for you !**

# Example

In the folder /AfdxSnmp_Arinc664, there is an example. What this example can do is to use the arinc664 board to send a snmp request to the target device, and then the snmp agent of the target device will return a response, and your host will receive this response To get the information you want.

The *l_SetAFDXTxQueue(...)* function in the *afdx_GenericTx.c* file is used to write the formed packet to the buffer. 

```c
//../AfdxSnmp_Arinc664/AfdxSnmp/afdx/afdx_GenericTx.c

AiInt32 l_SetTxQueue(AiUInt32 ul_Handle){
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

  printf("\n FdxCmdTxPortInit...");
  x_PortInitIn.ul_PortMap = 1;
  AIM_ASSERT( FdxCmdTxPortInit   (ul_Handle,&x_PortInitIn,&x_PortInitOut));
  printf("\n FdxCmdTxModeControl...");
  x_TxModeControl.ul_TransmitMode = FDX_TX_GENERIC;
  AIM_ASSERT( FdxCmdTxModeControl (ul_Handle,&x_TxModeControl));
  printf("\n FdxCmdTxQueueCreate...");
  x_TxQueueCreate.ul_QueueSize = 0;
  AIM_ASSERT( FdxCmdTxQueueCreate (ul_Handle,&x_TxQueueCreate,&x_TxQueueInfo));

  memset(&My_Frame.x_Frame, 0, sizeof(TY_FDX_TX_FRAME_HEADER) );
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
  My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize              = 64;
  My_Frame.x_Frame.x_FrameAttrib.uw_SequenceNumberInit     = FDX_TX_FRAME_SEQ_INIT_AUTO;
  My_Frame.x_Frame.x_FrameAttrib.uw_SequenceNumberOffset   = FDX_TX_FRAME_SEQ_OFFS_AUTO;
  // My_Frame.x_Frame.x_FrameAttrib.uc_TxIntEnable            = 3 /*FDX_ENA with Time Tag*/;

  /* Setup Frame */
  uc_FrameNo++;
  us_FrameCount = 1518;
  My_Frame.x_Frame.x_FrameAttrib.uw_FrameSize              = us_FrameCount;
  for (  i = 0 ; i<1600; i++)
    My_Frame.uc_Data[i] = uc_FrameNo;

  /* MAC */
  Dt[ 0]=0x03;Dt[ 1]=0x00;Dt[ 2]=0x00;Dt[ 3]=0x00;Dt[ 4]=0xff;Dt[ 5]=0x00;   /* Destination Address */
  Dt[ 6]=0x02;Dt[ 7]=0x00;Dt[ 8]=0x00;Dt[ 9]=0x01;Dt[10]=0x21;Dt[11]=0x20;   /* Source Address */
  Dt[12]=0x08;Dt[13]=0x00;                                                   /* Type */
  /* IP */
  Dt[14]=0x45;Dt[15]=0x00;
  Dt[16]=0x00;Dt[17]=0x2d;                           /* Ip Total Length */
  *((AiUInt16*)(&Dt[16])) =  BSWAP16_MAC(us_FrameCount -19);
  Dt[18]=0x00;Dt[19]=0x00;Dt[20]=0x40;Dt[21]=0x00;
  Dt[22]=0x01;Dt[23]=0x11;
  Dt[24]=0x00;Dt[25]=0x00;                           /* Byte 24 and 25 = Ip Header Check sum set to 0 for correct calculation*/
  Dt[26]=0x0a;Dt[27]=0x01;Dt[28]=0x21;Dt[29]=0x01;   /* Source Address */
  Dt[30]=0xe0;Dt[31]=0xe0;Dt[32]=0xff;Dt[33]=0x00;   /* Destinatin Address*/
  *((AiUInt16*)(&Dt[24])) = BSWAP16(us_CheckSum1ComplementSum((const AiUInt16 *)(&Dt[14]), 10/*Word count*/));
  /* UDP */
  Dt[34]=0x00;Dt[35]=0x01;Dt[36]=0x00;Dt[37]=0x02;   Dt[38]=0x00;Dt[39]=0x19;Dt[40]=0x00;Dt[41]=0x00;
  /* Data */
  sprintf((char*)&Dt[42],"F No. %d, %d Byte", uc_FrameNo, us_FrameCount); 

  for ( i = 0 ; i<60; i++)
    My_Frame.uc_Data[i] = (unsigned char) Dt[i];

  My_Frame.x_Frame.x_FrameAttrib.uc_NetSelect              = FDX_TX_FRAME_ONLY_B;

  printf("\n FdxCmdTxQueueWrite Frame %d ...", uc_FrameNo);
  AIM_ASSERT( FdxCmdTxQueueWrite (ul_Handle,FDX_TX_FRAME_HEADER_GENERIC,1,sizeof(My_Frame),&My_Frame));

  return(FDX_OK);
}
```

In the /* data */ section, you can replace it with any data you want to send. You need to replace Mac , IP and UDP Port.

In arinc664, mac address has different rules than Ethernet mac, please refer to the *ARINC664P7.pdf* file for details. This file is in the */doc* directory.

The board has many API interfaces that can be called, which are described in detail in the document *Arinc664_Reference_Manual.pdf*, and the document is in the */doc* directory.

Some of the interfaces used in the AfdxSnmp_Arinc664 project are as follows:

```c
AiReturn FdxCmdTxQueueWrite ( AiUInt32 ul_Handle, 
							  AiUInt32 ul_HeaderType,
							  AiUInt32 ul_EntryCount, 
							  AiUInt32 ul_WriteBytes, 
							  const void *pv_WriteBuffer);
AiReturn FdxCmdTxPortInit (AiUInt32 ul_Handle,
						   const TY_FDX_PORT_INIT_IN *px_PortInitIn,
						   TY_FDX_PORT_PORT_OUT *px_PortInitOut);
AiReturn FdxCmdTxModeControl( AiUInt32 ul_Handle,
						  	  const TY_FDX_TX_MODE_CTRL *px_TxModeControl);
AiReturn FdxCmdTxQueueCreate ( AiUInt32 ul_Handle,
							   const TY_FDX_TX_QUEUE_SETUP 
							   *px_TxQueueCreate,
							   TY_FDX_TX_QUEUE_INFO *px_TxQueueInfo);
AiReturn FdxCmdTxControl( AiUInt32 ul_Handle,
						  const TY_FDX_TX_CTRL *px_TxControl);
AiReturn FdxCmdTxStatus ( AiUInt32 ul_Handle,
						  TY_FDX_TX_STATUS *px_TxStatus);
```



# Contributing

Visual studio is an open source software, the snmp protocol can also be used for free, has been integrated in the windows system.

# License

Yes please! Feature requests / pull requests are welcome.

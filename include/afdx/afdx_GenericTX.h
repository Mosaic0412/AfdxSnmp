/******************************************************************************

Copyright (c) AIM GmbH 2007, 79111 Freiburg, Germany.

All rights reserved.  No part of this software may
be published, distributed, translated or otherwise
reproduced by any means or for any purpose without
the prior written consent of AIM GmbH, Freiburg.

-------------------------------------------------------------------------------

Description:

<Text>

-------------------------------------------------------------------------------

******************************************************************************/


/******************************************************************************
*  Includes
******************************************************************************/
#include <Ai_types.h>
#include <AiFdx_def_trg.h>
#include <afdx/afdx_types.h>


/******************************************************************************
*  local prototypes
******************************************************************************/
AiInt32 l_SetTxStaticRegisters(AiUInt32 ul_Handle);
AiInt32 l_SetTxQueue(AiUInt32 ul_Handle);
AiInt32 l_SetAFDXTxQueue(AiUInt32 ul_Handle, afdx_netsnmp_address_list *address, AiUInt snmp_data, AiUInt len);
AiInt32 l_SetTxQueueFragmented(AiUInt32 ul_Handle);
AiInt32 l_StartTxGetStatus(AiUInt32 ul_Handle1, AiUInt32 ul_Handle2, AiUInt32 ul_Cycles, AiUInt32 *paul_SubQueueHdl);
AiInt32 l_TransmitSetup(AiUInt32 ul_Handle1, AiUInt32 ul_Handle2);
AiInt32 l_TransmitAFDXSetup(AiUInt32 ul_Handle1, AiUInt32 ul_Handle2, afdx_netsnmp_address_list *address, AiUInt snmp_data, AiUInt len);
AiInt32 l_TestTxSimulateVl(AiUInt32 ul_Handle);
/* EOF */


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
    Functions for Login/Logout to AFDX board and port resources.

-------------------------------------------------------------------------------

******************************************************************************/


/******************************************************************************
 *  Includes
 ******************************************************************************/
#include <afdx/afdx_SampleUtils.h>
#include <afdx/afdx_LogInOut.h>
#include <afdx/afdx_types.h>

/******************************************************************************
 *  local Vars (Modul)
 ******************************************************************************/

/* structures for local resource administration */
TY_FDX_RESOURCE_INFO x_BoardResource = {0 , 0}; /* board resource for test */
TY_FDX_RESOURCE_INFO x_PortResource1 = {0 , 0}; /* standard port resource for test */
TY_FDX_RESOURCE_INFO x_PortResource2 = {0 , 0}; /* additional port resource */



/******************************************************************************
 *  extern Vars
 ******************************************************************************/


/******************************************************************************
 *  local prototypes
 ******************************************************************************/
AiUInt32 ul_EnterResourceID( void);

AiChar ServerName[128] = "local";
AiUInt32 ul_HandlePort1 = 0;
TY_SERVER_LIST *pServerList = NULL;

/*
 ##############################################################################
 #  Start of Code
 ##############################################################################
 */
FILE * pfCmdFile = NULL;

int ExtGetChar(void)
{
	int c;

	if (NULL != pfCmdFile)
	{
		c = fgetc(pfCmdFile);
		if (c == EOF)
		{
			fclose(pfCmdFile);
			pfCmdFile = NULL;
		}
	}
	else
	{
		c = getchar();
	}
	return c;
}
AiInt32 l_Init(){
	if (FdxInit(&pServerList) == FDX_OK)
	{
		AiUInt32 ul_HandleBoard = 0, ul_HandlePort1 = 0, ul_HandlePort2 = 0, ul_MemoryAccessHandle = 0;
		if (0 == ul_HandlePort1)
			l_LogIn(ServerName, &ul_HandlePort1, RESOURCETYPE_PORT);
		else
			l_LogIn(ServerName, &ul_HandlePort2, RESOURCETYPE_PORT);

		return FDX_ON;
	}

	fprintf(stderr, "\n\nFdxInit F.A.I.L.\n");
	return FDX_OFF;
}

/****************************************************************************
   Function    : l_LogIn
-----------------------------------------------------------------------------
   Create      : 
-----------------------------------------------------------------------------

   Inputs      : -

   Outputs     : -

   Description : .Testfunction for FdxLogin

*****************************************************************************/
AiInt32 l_LogIn(AiChar *ServerName, AiUInt32 *pul_Handle, AiUInt32 ul_ResourceType)
{
  AiInt32 l_RetVal = 0;
  AiUInt32 ul_ResourceId;
  AiInt16 w_LogedIn = 0;
  char Output[1024];
  AiBool32 bIsGnet = FALSE;;

  if (0 == *pul_Handle)
  {
    TY_RESOURCE_LIST_ELEMENT *ppx_ResourceList, *ppx_RLUse;

    AIM_ASSERT( FdxQueryServerConfig (ServerName,&ppx_ResourceList ));

    ul_ResourceId = ul_EnterResourceID( );

    ppx_RLUse = ppx_ResourceList;

    while ( (NULL != ppx_RLUse) && !w_LogedIn)
    {
      /* Check for APX */
      if (  ppx_RLUse->ul_ResourceType == FDX_BOARD_INFO)
      {
        if ( (ppx_RLUse->ac_ResourceInfo[0] == 'A') && 
             (ppx_RLUse->ac_ResourceInfo[1] == 'P') && 
             (ppx_RLUse->ac_ResourceInfo[2] == 'X') )
        {
          bIsGnet = TRUE; 
        }
        else 
        {
          bIsGnet = FALSE; 
        }
      }
      /* create list of available resources (selected type) */
      if (ppx_RLUse->ul_ResourceID == ul_ResourceId)
      {
        if (ul_ResourceType == ppx_RLUse->ul_ResourceType)
        {
          TY_FDX_CLIENT_INFO  x_ClientInfo;

          sprintf (x_ClientInfo.ac_ClApplication,         "Test" );
          sprintf (x_ClientInfo.ac_ClApplicationVersion,  "V01.00" );
          sprintf (x_ClientInfo.ac_ClHostName,            "local" );
          sprintf (x_ClientInfo.ac_ClUser,                "Fred" );

          AIM_ASSERT( FdxLogin(ServerName,&x_ClientInfo, ul_ResourceId, PRIVILEGES_ADMIN, pul_Handle));

          sprintf( Output, "\r\n FdxLogin O.K. Handle: %d", *pul_Handle);
          AIM_OUTPUT( Output);

          l_RetVal = 0;
          w_LogedIn = 1;
          /* store resource information for sample information */
          if ( RESOURCETYPE_BOARD == ppx_RLUse->ul_ResourceType)
          {
            x_BoardResource.ul_ID = ul_ResourceId;
            x_BoardResource.ul_Handle = *pul_Handle;
            sprintf(x_BoardResource.ac_Name, "%s", ppx_RLUse->ac_ResourceInfo);
            x_BoardResource.b_Gnet = bIsGnet;
          }
          else if (x_PortResource1.ul_ID == 0)
          {
            x_PortResource1.ul_ID = ul_ResourceId;
            x_PortResource1.ul_Handle = *pul_Handle;
            x_PortResource1.b_Gnet = bIsGnet;
          }
          else if (x_PortResource2.ul_ID == 0)
          {
            x_PortResource2.ul_ID = ul_ResourceId;
            x_PortResource2.ul_Handle = *pul_Handle;
            x_PortResource2.b_Gnet = bIsGnet;
          }
        }
        else
        {
          sprintf( Output, "\r\n Resource Type not valid.");
          AIM_OUTPUT( Output);
          l_RetVal = -1;
        }
      }

      ppx_RLUse = ppx_RLUse->px_Next;
    }

    if (!w_LogedIn)
    {
      sprintf( Output, "\r\n Resource Id not valid.");
      AIM_OUTPUT( Output);
      l_RetVal = -1;
    }

    if (NULL != ppx_ResourceList)
    {
      AIM_ASSERT( FdxCmdFreeMemory( ppx_ResourceList, ppx_ResourceList->ul_StructId));
    }
    else
    {
      sprintf( Output, "\r\n ppx_ResourceList Empty. FdxCmdFreeMemory() nothing to do.");
      AIM_OUTPUT( Output);
    }
  }
  else
  {
    sprintf( Output, "\r\n Already logged in.");
    AIM_OUTPUT( Output);

    l_RetVal = -1;
  }

  return (l_RetVal);
}



/****************************************************************************
   Function    : l_LogOut
-----------------------------------------------------------------------------
   Create      : 
-----------------------------------------------------------------------------

   Inputs      : -

   Outputs     : -

   Description : Testfunction for FdxLogout.

*****************************************************************************/
AiInt32 l_LogOut(AiUInt32 *pul_Handle, AiUInt32 ul_ResourceType)
{
  char Output[1024];
  AiUInt32 ul_Handle = *pul_Handle;
  
  AIM_ASSERT( FdxLogout(*pul_Handle));
  
  sprintf( Output, "\r\n FdxLogout O.K.");
  AIM_OUTPUT( Output);
  
  *pul_Handle = 0;

  if ( x_BoardResource.ul_Handle == ul_Handle) 
  {
    x_BoardResource.ul_ID = 0;
    x_BoardResource.ul_Handle = 0;
  }
  else if (x_PortResource1.ul_Handle == ul_Handle )
  {
    x_PortResource1.ul_ID = 0;
    x_PortResource1.ul_Handle = 0;
  }
  else if (x_PortResource2.ul_Handle == ul_Handle )
  {
    x_PortResource2.ul_ID = 0;
    x_PortResource2.ul_Handle = 0;
  }

  return (0);
}



/****************************************************************************
   Function    : l_TestQueryServerConfig
-----------------------------------------------------------------------------
   Create      : 24.06.2002 mr,AIM
-----------------------------------------------------------------------------

   Inputs      : -

   Outputs     : -

   Description : Testfunction for FdxQueryServerConfig.

*****************************************************************************/
AiInt32 l_TestQueryServerConfig( AiChar *ServerName)
{
  char Output[1024];
  AiUInt32 boardCnt, portCnt;

  TY_RESOURCE_LIST_ELEMENT *ppx_ResourceList, *ppx_RLUse;

  if(FdxQueryServerConfig (ServerName,&ppx_ResourceList ) == FDX_OK)
  {
    sprintf( Output, "\r\n FdxQueryServerConfig O.K. Server:%s", ServerName);
    AIM_OUTPUT( Output);
    sprintf( Output, "\r\n---ID-Type-Info------(List of available resources)-----------");
    AIM_OUTPUT( Output);

    boardCnt=0;
    portCnt=0;
    ppx_RLUse = ppx_ResourceList;
    while ( NULL != ppx_RLUse)
    {
      if ((x_BoardResource.ul_ID == ppx_RLUse->ul_ResourceID) || 
          (x_PortResource1.ul_ID == ppx_RLUse->ul_ResourceID)   )
      {
        sprintf( Output, "\r\n * %2d %4d "  , ppx_RLUse->ul_ResourceID, ppx_RLUse->ul_ResourceType);
        AIM_OUTPUT( Output);
      }
      else if ( x_PortResource2.ul_ID == ppx_RLUse->ul_ResourceID)
      {
        sprintf( Output, "\r\n(*)%2d %4d "  , ppx_RLUse->ul_ResourceID, ppx_RLUse->ul_ResourceType);
        AIM_OUTPUT( Output);
      }
      else 
      {
        sprintf( Output, "\r\n   %2d %4d "  , ppx_RLUse->ul_ResourceID, ppx_RLUse->ul_ResourceType);
        AIM_OUTPUT( Output);
      }

      if (RESOURCETYPE_BOARD == ppx_RLUse->ul_ResourceType)
      {
        TY_FDX_BOARD_RESOURCE x_ResourceInfo;

        AIM_ASSERT( FdxQueryResource (ServerName, ppx_RLUse->ul_ResourceID, &x_ResourceInfo ));

        /* store ResourceIdentifier of board for login */
        boardCnt++;

        sprintf( Output, "BoardName:%s ", x_ResourceInfo.ac_BoardName);
        AIM_OUTPUT( Output);
        sprintf( Output, "BoardSerialNo:%d ", x_ResourceInfo.ul_BoardSerialNo);
        AIM_OUTPUT( Output);
      }
      else
      {
        TY_FDX_PORT_RESOURCE x_ResourceInfo;

        AIM_ASSERT( FdxQueryResource (ServerName, ppx_RLUse->ul_ResourceID, &x_ResourceInfo ));

        /* store ResourceIdentifier of port for login */
        portCnt++;

        sprintf( Output, "PortName:%s ", x_ResourceInfo.ac_PortName);
        AIM_OUTPUT( Output);
        sprintf( Output, "PortNo:%d ", x_ResourceInfo.uc_PortNo);
        AIM_OUTPUT( Output);
        sprintf( Output, "PortMode:%d ", x_ResourceInfo.uc_PortMode);
        AIM_OUTPUT( Output);

      }

      ppx_RLUse = ppx_RLUse->px_Next;
    }
    sprintf( Output, "\r\n-(End of resource list)------------------------------------");
    AIM_OUTPUT( Output);
    sprintf( Output, "\r\nNOTE: Login to required resources before using functions!!!");
    AIM_OUTPUT( Output);

    if (NULL != ppx_ResourceList)
    {
      AIM_ASSERT( FdxCmdFreeMemory( ppx_ResourceList, ppx_ResourceList->ul_StructId));
    }
    else
    {
      sprintf( Output, "\r\n ppx_ResourceList Empty. FdxCmdFreeMemory() nothing to do.");
      AIM_OUTPUT( Output);
    }
  }

  return (0);
}


/****************************************************************************
   Function    : ul_EnterResourceID
-----------------------------------------------------------------------------
   Create      : 
-----------------------------------------------------------------------------

   Inputs      : -

   Outputs     : -

   Description : Function for User Input
                 (Select Resource ID number)

*****************************************************************************/
AiUInt32 ul_EnterResourceID( void)
{
  AiUInt32 ul_ResourceId;
  char Output[1024];

  sprintf( Output, "\r\nSelect Resource ID:>");
  AIM_OUTPUT(Output);
  do
  {
    ul_ResourceId = ExtGetChar();
  }
  while (10 == ul_ResourceId);  /* to eliminate <cr> */
  ul_ResourceId -= 48;          /* make integer from ASCII */
  sprintf( Output, "\r\nResource ID:%d", ul_ResourceId);
  AIM_OUTPUT(Output);

  return ( ul_ResourceId);
}



/****************************************************************************
   Function    : b_IsGnetHandle
-----------------------------------------------------------------------------
   Create      : 
-----------------------------------------------------------------------------

   Inputs      : -

   Outputs     : -

   Description : 

*****************************************************************************/
AiBool32 b_IsGnetHandle(AiUInt32 ul_Handle)
{
   AiBoolean IsApeAxcBoard = 0;

   FdxIsApeAxcBoard(ul_Handle, &IsApeAxcBoard);
   if (TRUE == IsApeAxcBoard)
       return TRUE;
   if (ul_Handle == x_BoardResource.ul_Handle)
      return  x_BoardResource.b_Gnet;
   else if (ul_Handle == x_PortResource1.ul_Handle)
      return x_PortResource1.b_Gnet;
   else if (ul_Handle == x_PortResource1.ul_Handle)
      return x_PortResource1.b_Gnet;
   else
      return FALSE;
}
/* EOF */


/******************************************************************************

    Copyright (c) AIM GmbH 2007, 79111 Freiburg, Germany.

        All rights reserved.  No part of this software may
        be published, distributed, translated or otherwise
        reproduced by any means or for any purpose without
        the prior written consent of AIM GmbH, Freiburg.

-------------------------------------------------------------------------------

    Description:

    <Text>

------------------------------------------------------------------------------

******************************************************************************/


/******************************************************************************
 *  Includes
******************************************************************************/
#include "AiFdx_def.h"

#include <afdx/afdx_SampleUtils.h>

/******************************************************************************
 *  Defines
******************************************************************************/

/******************************************************************************
 *  local Vars (Modul)
******************************************************************************/


/******************************************************************************
 *  extern Vars
******************************************************************************/


/******************************************************************************
 *  local prototypes
******************************************************************************/



/*
 ##############################################################################
 #  Start of Code
 ##############################################################################
 */

/* --------------------------------------------------------------------------- 
   Global Functions
   ------------------------------------------------------------------------- */

   
/****************************************************************************
   Function    : Usage
-----------------------------------------------------------------------------
   Create      : 
-----------------------------------------------------------------------------

   Inputs      : -

   Outputs     : -

   Description : .

*****************************************************************************/
void Usage(char * pszProgramName)
{
  fprintf(stderr, "Usage:  %s\n", pszProgramName);
  fprintf(stderr, " -s Server Name\n");
  printf("Press any key to continue...\n");

  GetConsoleCharacter();

  exit(1);
}



/****************************************************************************
   Function    : ul_GenerateIp
-----------------------------------------------------------------------------
   Create      : 8.07.2002 rh
-----------------------------------------------------------------------------

   Inputs      : -

   Outputs     : -

   Description : Utilityfunction To generate a hex Ip-Address from a address string

*****************************************************************************/
AiUInt32 ul_GenerateIp(const char *IpString )
{
  union{
    AiUInt32 ul_HexIpAddress;
    AiUInt8  uc_AddressPart[4];
  }x_IpAddress;

  int i,j;
  size_t length;
  size_t index;

  if (*IpString == '"')
  {
    IpString++;
    length = strlen(IpString);
    index  = length-1;
    if ( IpString[index] == '"')
      index--;
  }
  else
  {
    length = strlen(IpString);
    index  = length-1;
  }


#if defined(HOST_ENDIAN_BIG)
  for (i=3; i>=0; i--)
#else
    for (i=0; i<4; i++)
#endif
    {
      j = 0;
      x_IpAddress.uc_AddressPart[i] = 0;

      while ( (index >= 0) && (IpString[index] != '.') )
      {
        switch (j)
        {
        case 0:  x_IpAddress.uc_AddressPart[i] += ( (IpString[index]-0x30)      ); break;
        case 1:  x_IpAddress.uc_AddressPart[i] += ( (IpString[index]-0x30) *  10); break;
        case 2:  x_IpAddress.uc_AddressPart[i] += ( (IpString[index]-0x30) * 100); break;
        }
        j++;
        if (index == 0)
          break;
        else
          index--;
      }
      index--;

    }
    return(x_IpAddress.ul_HexIpAddress);
}



/****************************************************************************
   Function    : ul_GenAFDXUnicastIp
-----------------------------------------------------------------------------
   Create      : 22.08.2002 rh
-----------------------------------------------------------------------------

   Inputs      : -

   Outputs     : -

   Description : Utilityfunction To generate a hex Ip-Address (unicast) from 
                 the specified AFDX parameters.

*****************************************************************************/
AiUInt32 ul_GenAFDXUnicastIp( AiUInt8 uc_DomainId, AiUInt8 uc_SideId, AiUInt8 uc_LocationId, AiUInt8 uc_PartitionId )
{
  AiUInt32 ul_IpAddress;

  ul_IpAddress  = 0x0A800000;
  ul_IpAddress |= ( ((((AiUInt32)uc_DomainId    )) << 16 ) & 0x000F0000);
  ul_IpAddress |= ( ((((AiUInt32)uc_SideId      )) << 13 ) & 0x0000E000);
  ul_IpAddress |= ( ((((AiUInt32)uc_LocationId  )) <<  8 ) & 0x00001F00);
  ul_IpAddress |= ( ((((AiUInt32)uc_PartitionId )) <<  0 ) & 0x000F001F);

  return (ul_IpAddress);
}



/****************************************************************************
   Function    : ul_GenAFDXMulticastIp
-----------------------------------------------------------------------------
   Create      : 22.08.2002 rh
-----------------------------------------------------------------------------

   Inputs      : -

   Outputs     : -

   Description : Utilityfunction To generate a hex Ip-Address (multicast) 
                 which contains the AFDX VL-Id

*****************************************************************************/
AiUInt32 ul_GenAFDXMulticastIp(AiUInt16 uw_Vl)
{
  AiUInt32 ul_IpAddress;

  ul_IpAddress  = 0xE0E00000;
  ul_IpAddress += uw_Vl;

  return (ul_IpAddress);
}


void v_GenerateIPString(const AiUInt32 ul_IpAddr, char* pc_IpStr)
{
    sprintf(pc_IpStr, "%d.%d.%d.%d", (AiUInt8)(ul_IpAddr >> 24),
            (AiUInt8)((ul_IpAddr >> 16) & 0x000000FF),
            (AiUInt8)((ul_IpAddr >> 8) & 0x000000FF),
            (AiUInt8)(ul_IpAddr & 0x000000FF));
}


/****************************************************************************
   Function    : us_CheckSum1ComplementSum
-----------------------------------------------------------------------------
   Create      : 
-----------------------------------------------------------------------------

   Inputs      : -

   Outputs     : -

   Description : see http://www.netrino.com/Connecting/1999-11/ for the 
                 "original" of this function -> NetIpChecksum

*****************************************************************************/
AiUInt16 us_CheckSum1ComplementSum(const AiUInt16 * pW, AiInt32 iWordCnt)
{
  AiUInt32  sum = 0;
  AiInt32 i;

  /* IP headers always contain an even number of bytes. */
  for (i=0; i<iWordCnt; i++)
  {
    /* sum += _BSWAP16(pW[i]); */
    sum += pW[i];
  }

  /* Use carries to compute 1's complement sum. */
  sum = (sum >> 16) + (sum & 0xFFFF);
  sum += sum >> 16;

  /* Return the inverted 16-bit result. */
  return ((AiUInt16) ~sum);
}


char GetConsoleCharacter(void)
{
    char ch = 0;
    int  iChar;
    int count = 0;

    do
    {
        iChar = getchar();
        if ((iChar != '\n') && (iChar != EOF))
            ch = (char)iChar;
        else
        {
            if (count > 0)
                break;
        }
        count++;
    } while (1);

    return ch;
}

void GetConsoleString(char buffer[], int maxLen)
{
    int ch, i;

    for (i = 0; (i < maxLen) && ((ch = getchar()) != EOF)
         && (ch != '\n'); i++)
    {
        buffer[i] = (char)ch;
    }

    // Terminate string with a null character 
    buffer[i] = '\0';
}

/* EOF */


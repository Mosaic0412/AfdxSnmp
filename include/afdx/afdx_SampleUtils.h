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
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "AiOs.h"
#include "AiFdx_def.h"

/* some system specific includes */

#ifdef _AIM_LINUX
   #include <unistd.h>
#endif

#ifdef __LYNX_OS__
  #include <unistd.h>
#endif

#ifdef __VXWORKS__
  #include <vxWorks.h>
  #include <tasklib.h>
  #include <semlib.h>

  #include <intlib.h>
  #include <syslib.h>

  #include <unistd.h>
  #include <ctype.h>
#endif

/******************************************************************************
 *  Defines
******************************************************************************/
#define MODULINFO             __FILE__, __DATE__, __TIME__, __LINE__
#define AIM_OUTPUT(STRING)    printf("%s", STRING)
#define AIM_ERR_OUTPUT(ERR)   printf("%s,%s,%s,line:%d, Return Value (%d)\n", MODULINFO, ERR)
#define AIM_OK_RESULT         FDX_OK

#ifdef __VXWORKS__ 
#define AIM_ASSERT(API_FKT){ \
                             AiReturn ret; \
                             if (AIM_OK_RESULT != (ret=API_FKT)) \
                             { \
                               AIM_ERR_OUTPUT(ret); \
                               return (ret); \
                             } \
                           }
#else 
#ifndef __cplusplus
#define AIM_ASSERT(API_FKT)     {     \
                                    AiReturn ret; \
                                    if (AIM_OK_RESULT != (ret=API_FKT)) \
                                    { \
                                        AIM_ERR_OUTPUT(ret); \
                                        return (ret); \
                                    } \
                                }
#else 
#define AIM_ASSERT(API_FKT)     {     \
                                    AiReturn ret; \
                                    if (AIM_OK_RESULT != (ret=API_FKT)) \
                                    { \
                                        AIM_ERR_OUTPUT(ret); \
                                        throw (AiReturn)-1; \
                                    } \
                                }
#endif
#endif


#ifdef __LYNX_OS__                           /*07.02.2002 MR LynxOS */
  #define AIM_WAIT(x) usleep(x*1000)
#endif
#ifdef _AIM_LINUX
  #define AIM_WAIT(x) usleep(x*1000)
#endif
#ifdef __VXWORKS__
  #define AIM_WAIT(x) taskDelay(sysClkRateGet()* x/1000)
/*  #define AIM_WAIT(x) taskDelay(x/15) */
#endif

#ifdef _AIM_WINDOWS
  #define AIM_WAIT(x) Sleep(x)
#endif
#ifdef _AIM_VXI
  #define AIM_WAIT(x) Sleep(x)
#endif



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

void Usage(char * pszProgramName);
int ExtGetChar( void );

AiUInt32 ul_GenerateIp(const char *IpString );
AiUInt32 ul_GenAFDXUnicastIp(AiUInt8 uc_DomainId, AiUInt8 uc_SideId, AiUInt8 uc_LocationId, AiUInt8 uc_PartitionId);
AiUInt32 ul_GenAFDXMulticastIp(AiUInt16 uw_Vl);
AiUInt16 us_CheckSum1ComplementSum(const AiUInt16 * pW, AiInt32 iWordCnt);

char GetConsoleCharacter(void);
void GetConsoleString(char buffer[], int maxLen);



/* EOF */


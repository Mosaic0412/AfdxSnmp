 /*
 * mt_support.c - multi-thread resource locking support 
 */
/*
 * Author: Markku Laukkanen
 * Created: 6-Sep-1999
 * History:
 *  8-Sep-1999 M. Slifcak method names changed;
 *                        use array of resource locking structures.
 */

#include <net-snmp-config.h>
#include <errno.h>
#include <library/mt_support.h>


#ifdef WIN32

/*
 * Provide "do nothing" targets for Release (.DLL) builds. 
 */

int
snmp_res_init(void)
{
    return 0;
}

int
snmp_res_lock(int groupID, int resourceID)
{
    return 0;
}

int
snmp_res_unlock(int groupID, int resourceID)
{
    return 0;
}

int
snmp_res_destroy_mutex(int groupID, int resourceID)
{
    return 0;
}
#endif /*  WIN32  */

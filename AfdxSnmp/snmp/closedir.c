/*
 * closedir() replacement for MSVC.
 */

#define WIN32IO_IS_STDIO

#include <net-snmp-config.h>
#include <types.h>
#include <library/system.h>

/*
 * free the memory allocated by opendir 
 */
int
closedir(DIR * dirp)
{
    free(dirp->start);
    free(dirp);
    return 1;
}

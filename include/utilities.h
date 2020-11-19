#ifndef NET_SNMP_UTILITIES_H
#define NET_SNMP_UTILITIES_H

/**
*  Library API routines not specifically concerned with SNMP directly,
*    but used more generally within the library, agent and other applications.
*
*  This also includes "standard" system routines, which are missing on
*    particular O/S distributiones.
*/

#include <net-snmp-config.h>
#include <types.h>

/*
*  For the initial release, this will just refer to the
*  relevant UCD header files.
*    In due course, the routines relevant to this area of the
*  API will be identified, and listed here directly.
*
*  But for the time being, this header file is a placeholder,
*  to allow application writers to adopt the new header file names.
*/

#include <library/snmp_api.h>
#include <library/snmp_client.h>
#if HAVE_GETOPT_H
#include <getopt.h>
#else
#include <library/getopt.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>         /* for in_addr_t */
#endif
#include <library/system.h>
#include <library/tools.h>
#include <library/asn1.h>      /* for counter64 */
#include <library/int64.h>

#include <library/mt_support.h>
/*  #include <library/snmp_locking.h>  */
#include <library/snmp_alarm.h>
#include <library/callback.h>
#include <library/data_list.h>
#include <library/oid_stash.h>
#include <library/snmp.h>
#include <library/snmp_impl.h>
#include <library/snmp-tc.h>
#include <library/check_varbind.h>
#include <library/container.h>
#include <library/container_binary_array.h>
#include <library/container_list_ssll.h>
#include <library/container_iterator.h>

#include <library/snmp_assert.h>

#endif                          /* NET_SNMP_UTILITIES_H */

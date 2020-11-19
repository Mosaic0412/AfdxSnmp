/*
 * system.c
 */
/* Portions of this file are subject to the following copyright(s).  See
 * the Net-SNMP's COPYING file for more details and other copyrights
 * that may apply:
 */
/***********************************************************
        Copyright 1992 by Carnegie Mellon University

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of CMU not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

CMU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/
/*
 * Portions of this file are copyrighted by:
 * Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */
/*
 * Portions of this file are copyrighted by:
 * Copyright (C) 2007 Apple, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */
/*
 * System dependent routines go here
 */
#include <net-snmp-config.h>
#include <net-snmp-features.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#include <io.h>
#include <direct.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <limits.h>

#include <types.h>
#include <output_api.h>
#include <utilities.h>
#include <library/system.h>    /* for "internal" definitions */

#include <library/snmp_api.h>
#include <library/read_config.h> /* for get_temp_file_pattern() */

#include "inet_ntop.h"

#include <library/winperf.h>

/* NetSNMP and DNSSEC-Tools both define FREE. We'll not use either here. */
#undef FREE

netsnmp_feature_child_of(system_all, libnetsnmp)

netsnmp_feature_child_of(user_information, system_all)
netsnmp_feature_child_of(calculate_sectime_diff, system_all)

# define LOOPBACK    INADDR_LOOPBACK

/**
 * fork current process into the background.
 *
 * This function forks a process into the background, in order to
 * become a daemon process. It does a few things along the way:
 *
 * - becoming a process/session group leader, and  forking a second time so
 *   that process/session group leader can exit.
 *
 * - changing the working directory to /
 *
 * - closing stdin, stdout and stderr (unless stderr_log is set) and
 *   redirecting them to /dev/null
 *
 * @param quit_immediately : indicates if the parent process should
 *                           exit after a successful fork.
 * @param stderr_log       : indicates if stderr is being used for
 *                           logging and shouldn't be closed
 * @returns -1 : fork error
 *           0 : child process returning
 *          >0 : parent process returning. returned value is the child PID.
 */
int
netsnmp_daemonize(int quit_immediately, int stderr_log)
{
    int i = 0;
    DEBUGMSGT(("daemonize","deamonizing...\n"));
    return i;
}

/*
 * ********************************************* 
 */
#ifdef							WIN32
in_addr_t
get_myaddr(void)
{
    char            local_host[130];
    int             result;
    LPHOSTENT       lpstHostent;
    SOCKADDR_IN     in_addr, remote_in_addr;
    SOCKET          hSock;
    int             nAddrSize = sizeof(SOCKADDR);

    in_addr.sin_addr.s_addr = INADDR_ANY;

    result = gethostname(local_host, sizeof(local_host));
    if (result == 0) {
        lpstHostent = gethostbyname((LPSTR) local_host);
        if (lpstHostent) {
            in_addr.sin_addr.s_addr =
                *((u_long FAR *) (lpstHostent->h_addr));
            return ((in_addr_t) in_addr.sin_addr.s_addr);
        }
    }

    /*
     * if we are here, than we don't have host addr 
     */
    hSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (hSock != INVALID_SOCKET) {
        /*
         * connect to any port and address 
         */
        remote_in_addr.sin_family = AF_INET;
        remote_in_addr.sin_port = htons(IPPORT_ECHO);
        remote_in_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
        result =
            connect(hSock, (LPSOCKADDR) & remote_in_addr,
                    sizeof(SOCKADDR));
        if (result != SOCKET_ERROR) {
            /*
             * get local ip address 
             */
            getsockname(hSock, (LPSOCKADDR) & in_addr,
                        (int FAR *) &nAddrSize);
        }
        closesocket(hSock);
    }
    return ((in_addr_t) in_addr.sin_addr.s_addr);
}

long
get_uptime(void)
{
    long            return_value = 0;
    DWORD           buffersize = (sizeof(PERF_DATA_BLOCK) +
                                  sizeof(PERF_OBJECT_TYPE)),
        type = REG_EXPAND_SZ;
    PPERF_DATA_BLOCK perfdata = NULL;

    /*
     * min requirement is one PERF_DATA_BLOCK plus one PERF_OBJECT_TYPE 
     */
    perfdata = (PPERF_DATA_BLOCK) malloc(buffersize);
    if (!perfdata)
        return 0;

    memset(perfdata, 0, buffersize);

    RegQueryValueEx(HKEY_PERFORMANCE_DATA,
                    "Global", NULL, &type, (LPBYTE) perfdata, &buffersize);

    /*
     * we can not rely on the return value since there is always more so
     * we check the signature 
     */

    if (wcsncmp(perfdata->Signature, L"PERF", 4) == 0) {
        /*
         * signature ok, and all we need is in the in the PERF_DATA_BLOCK 
         */
        return_value = (long) ((perfdata->PerfTime100nSec.QuadPart /
                                (LONGLONG) 100000));
    } else
        return_value = GetTickCount() / 10;

    RegCloseKey(HKEY_PERFORMANCE_DATA);
    free(perfdata);

    return return_value;
}

char           *
winsock_startup(void)
{
    WORD            VersionRequested;
    WSADATA         stWSAData;
    int             i;
    static char     errmsg[100];

	/* winsock 1: use MAKEWORD(1,1) */
	/* winsock 2: use MAKEWORD(2,2) */

    VersionRequested = MAKEWORD(2,2);
    i = WSAStartup(VersionRequested, &stWSAData);
    if (i != 0) {
        if (i == WSAVERNOTSUPPORTED)
            sprintf(errmsg,
                    "Unable to init. socket lib, does not support 1.1");
        else {
            sprintf(errmsg, "Socket Startup error %d", i);
        }
        return (errmsg);
    }
    return (NULL);
}

void
winsock_cleanup(void)
{
    WSACleanup();
}

#endif                          /* ! WIN32 */
/*******************************************************************/

int
netsnmp_gethostbyname_v4(const char* name, in_addr_t *addr_out)
{
    struct hostent *hp = NULL;

    hp = netsnmp_gethostbyname(name);
    if (hp == NULL) {
        DEBUGMSGTL(("get_thisaddr",
                    "hostname (couldn't resolve)\n"));
        return -1;
    } else if (hp->h_addrtype != AF_INET) {
        DEBUGMSGTL(("get_thisaddr",
                    "hostname (not AF_INET!)\n"));
        return -1;
    } else {
        DEBUGMSGTL(("get_thisaddr",
                    "hostname (resolved okay)\n"));
        memcpy(addr_out, hp->h_addr, sizeof(in_addr_t));
    }
    return 0;
}

int
netsnmp_getaddrinfo(const char *name, const char *service,
                    const struct addrinfo *hints, struct addrinfo **res)
{
    NETSNMP_LOGONCE((LOG_ERR, "getaddrinfo not available"));
    return EAI_FAIL;
}

struct hostent *
netsnmp_gethostbyname(const char *name)
{
#if HAVE_GETHOSTBYNAME
    struct hostent *hp = NULL;

    if (NULL == name)
        return NULL;

    DEBUGMSGTL(("dns:gethostbyname", "looking up %s\n", name));

    hp = gethostbyname(name);
    if (hp == NULL) {
        DEBUGMSGTL(("dns:gethostbyname",
                    "couldn't resolve %s\n", name));
    } else if (hp->h_addrtype != AF_INET) {
        DEBUGMSGTL(("dns:gethostbyname",
                    "warning: response for %s not AF_INET!\n", name));
    } else {
        DEBUGMSGTL(("dns:gethostbyname",
                    "%s resolved okay\n", name));
    }
    return hp;
#endif /* HAVE_GETHOSTBYNAME */
}

/**
 * Look up the host name via DNS.
 *
 * @param[in] addr Pointer to the address to resolve. This argument points e.g.
 *   to a struct in_addr for AF_INET or to a struct in6_addr for AF_INET6.
 * @param[in] len  Length in bytes of *addr.
 * @param[in] type Address family, e.g. AF_INET or AF_INET6.
 *
 * @return Pointer to a hostent structure if address lookup succeeded or NULL
 *   if the lookup failed.
 *
 * @see See also the gethostbyaddr() man page.
 */
struct hostent *
netsnmp_gethostbyaddr(const void *addr, socklen_t len, int type)
{
#if HAVE_GETHOSTBYADDR
    struct hostent *hp = NULL;
    char buf[64];

    DEBUGMSGTL(("dns:gethostbyaddr", "resolving %s\n",
                inet_ntop(type, addr, buf, sizeof(buf))));

    hp = gethostbyaddr(addr, len, type);
    if (hp == NULL) {
        DEBUGMSGTL(("dns:gethostbyaddr", "couldn't resolve addr\n"));
    } else if (hp->h_addrtype != AF_INET) {
        DEBUGMSGTL(("dns:gethostbyaddr",
                    "warning: response for addr not AF_INET!\n"));
    } else {
        DEBUGMSGTL(("dns:gethostbyaddr", "addr resolved okay\n"));
    }
    return hp;
#endif
}

/*******************************************************************/

#ifndef HAVE_STRNCASECMP

/*
 * test for NULL pointers before and NULL characters after
 * * comparing possibly non-NULL strings.
 * * WARNING: This function does NOT check for array overflow.
 */
int
strncasecmp(const char *s1, const char *s2, size_t nch)
{
    size_t          ii;
    int             res = -1;

    if (!s1) {
        if (!s2)
            return 0;
        return (-1);
    }
    if (!s2)
        return (1);

    for (ii = 0; (ii < nch) && *s1 && *s2; ii++, s1++, s2++) {
        res = (int) (tolower(*s1) - tolower(*s2));
        if (res != 0)
            break;
    }

    if (ii == nch) {
        s1--;
        s2--;
    }

    if (!*s1) {
        if (!*s2)
            return 0;
        return (-1);
    }
    if (!*s2)
        return (1);

    return (res);
}

int
strcasecmp(const char *s1, const char *s2)
{
    return strncasecmp(s1, s2, 1000000);
}

#endif                          /* HAVE_STRNCASECMP */

#ifndef HAVE_SETENV
int
setenv(const char *name, const char *value, int overwrite)
{
    char           *cp;
    int             ret;

    if (overwrite == 0) {
        if (getenv(name))
            return 0;
    }
    cp = (char *) malloc(strlen(name) + strlen(value) + 2);
    if (cp == NULL)
        return -1;
    sprintf(cp, "%s=%s", name, value);
    ret = putenv(cp);
#ifdef WIN32
    free(cp);
#endif
    return ret;
}
#endif                          /* HAVE_SETENV */

netsnmp_feature_child_of(calculate_time_diff, netsnmp_unused)
#ifndef NETSNMP_FEATURE_REMOVE_CALCULATE_TIME_DIFF
/**
 * Compute (*now - *then) in centiseconds.
 */
int
calculate_time_diff(const struct timeval *now, const struct timeval *then)
{
    struct timeval  diff;

    NETSNMP_TIMERSUB(now, then, &diff);
    return (int)(diff.tv_sec * 100 + diff.tv_usec / 10000);
}
#endif /* NETSNMP_FEATURE_REMOVE_CALCULATE_TIME_DIFF */

#ifndef NETSNMP_FEATURE_REMOVE_CALCULATE_SECTIME_DIFF
/** Compute rounded (*now - *then) in seconds. */
u_int
calculate_sectime_diff(const struct timeval *now, const struct timeval *then)
{
    struct timeval  diff;

    NETSNMP_TIMERSUB(now, then, &diff);
    return diff.tv_sec + (diff.tv_usec >= 500000L);
}
#endif /* NETSNMP_FEATURE_REMOVE_CALCULATE_SECTIME_DIFF */

#ifndef HAVE_STRCASESTR
/*
 * only glibc2 has this.
 */
char           *
strcasestr(const char *haystack, const char *needle)
{
    const char     *cp1 = haystack, *cp2 = needle;
    const char     *cx;
    int             tstch1, tstch2;

    /*
     * printf("looking for '%s' in '%s'\n", needle, haystack); 
     */
    if (cp1 && cp2 && *cp1 && *cp2)
        for (cp1 = haystack, cp2 = needle; *cp1;) {
            cx = cp1;
            cp2 = needle;
            do {
                /*
                 * printf("T'%c' ", *cp1); 
                 */
                if (!*cp2) {    /* found the needle */
                    /*
                     * printf("\nfound '%s' in '%s'\n", needle, cx); 
                     */
                    return NETSNMP_REMOVE_CONST(char *, cx);
                }
                if (!*cp1)
                    break;

                tstch1 = toupper(*cp1);
                tstch2 = toupper(*cp2);
                if (tstch1 != tstch2)
                    break;
                /*
                 * printf("M'%c' ", *cp1); 
                 */
                cp1++;
                cp2++;
            }
            while (1);
            if (*cp1)
                cp1++;
        }
    /*
     * printf("\n"); 
     */
    if (cp1 && *cp1)
        return NETSNMP_REMOVE_CONST(char *, cp1);

    return NULL;
}
#endif

int
mkdirhier(const char *pathname, mode_t mode, int skiplast)
{
    struct stat     sbuf;
    char           *ourcopy = strdup(pathname);
    char           *entry;
    char           *buf = NULL;
    char           *st = NULL;
    int             res;

    res = SNMPERR_GENERR;
    if (!ourcopy)
        goto out;

    buf = malloc(strlen(pathname) + 2);
    if (!buf)
        goto out;

#if defined (WIN32) || defined (cygwin)
    /* convert backslash to forward slash */
    for (entry = ourcopy; *entry; entry++)
        if (*entry == '\\')
            *entry = '/';
#endif

    entry = strtok_r(ourcopy, "/", &st);

    buf[0] = '\0';

#if defined (WIN32) || defined (cygwin)
    /*
     * Check if first entry contains a drive-letter
     *   e.g  "c:/path"
     */
    if ((entry) && (':' == entry[1]) &&
        (('\0' == entry[2]) || ('/' == entry[2]))) {
        strcat(buf, entry);
        entry = strtok_r(NULL, "/", &st);
    }
#endif

    /*
     * check to see if filename is a directory 
     */
    while (entry) {
        strcat(buf, "/");
        strcat(buf, entry);
        entry = strtok_r(NULL, "/", &st);
        if (entry == NULL && skiplast)
            break;
        if (stat(buf, &sbuf) < 0) {
            /*
             * DNE, make it 
             */
#ifdef WIN32
            if (CreateDirectory(buf, NULL) == 0)
#else
            if (mkdir(buf, mode) == -1)
#endif
                goto out;
            else
                snmp_log(LOG_INFO, "Created directory: %s\n", buf);
        } else {
            /*
             * exists, is it a file? 
             */
            if ((sbuf.st_mode & S_IFDIR) == 0) {
                /*
                 * ack! can't make a directory on top of a file 
                 */
                goto out;
            }
        }
    }
    res = SNMPERR_SUCCESS;
out:
    free(buf);
    free(ourcopy);
    return res;
}

/**
 * netsnmp_mktemp creates a temporary file based on the
 *                 configured tempFilePattern
 *
 * @return file descriptor
 */
const char     *
netsnmp_mktemp(void)
{
    static char     name[256];
    int             fd = -1;

    strlcpy(name, get_temp_file_pattern(), sizeof(name));

    if (mktemp(name)) {
        /*
         * Win32 needs _S_IREAD | _S_IWRITE to set permissions on file
         * after closing
         */
        fd = _open(name, _O_CREAT | _O_EXCL | _O_WRONLY, _S_IREAD | _S_IWRITE);
    }
    if (fd >= 0) {
        close(fd);
        DEBUGMSGTL(("netsnmp_mktemp", "temp file created: %s\n",
                    name));
        return name;
    }
    snmp_log(LOG_ERR, "netsnmp_mktemp: error creating file %s\n",
             name);
    return NULL;
}

/*
 * This function was created to differentiate actions
 * that are appropriate for Linux 2.4 kernels, but not later kernels.
 *
 * This function can be used to test kernels on any platform that supports uname().
 *
 * If not running a platform that supports uname(), return -1.
 *
 * If ospname matches, and the release matches up through the prefix,
 *  return 0.
 * If the release is ordered higher, return 1.
 * Be aware that "ordered higher" is not a guarantee of correctness.
 */
int
netsnmp_os_prematch(const char *ospmname,
                    const char *ospmrelprefix)
{
  return -1;
}

/**
 * netsnmp_os_kernel_width determines kernel width at runtime
 * Currently implemented for IRIX, AIX and Tru64 Unix
 *
 * @return kernel width (usually 32 or 64) on success, -1 on error
 */
int
netsnmp_os_kernel_width(void)
{
  return -1;
}

netsnmp_feature_child_of(str_to_uid, user_information)
#ifndef NETSNMP_FEATURE_REMOVE_STR_TO_UID
/**
 * Convert a user name or number into numeric form.
 *
 * @param[in] useroruid Either a Unix user name or the ASCII representation
 *   of a user number.
 *
 * @return Either a user number > 0 or 0 if useroruid is not a valid user
 *   name, not a valid user number or the name of the root user.
 */
int netsnmp_str_to_uid(const char *useroruid) {
    int uid;
    uid = atoi(useroruid);
    if (uid == 0) {
        if (uid == 0)
            snmp_log(LOG_WARNING, "Can't identify user (%s).\n", useroruid);
    }
    return uid;
}
#endif /* NETSNMP_FEATURE_REMOVE_STR_TO_UID */

netsnmp_feature_child_of(str_to_gid, user_information)
#ifndef NETSNMP_FEATURE_REMOVE_STR_TO_GID
/**
 * Convert a group name or number into numeric form.
 *
 * @param[in] grouporgid Either a Unix group name or the ASCII representation
 *   of a group number.
 *
 * @return Either a group number > 0 or 0 if grouporgid is not a valid group
 *   name, not a valid group number or the root group.
 */
int netsnmp_str_to_gid(const char *grouporgid)
{
    int gid;

    gid = atoi(grouporgid);

    if (gid == 0) {

        if (gid == 0)
            snmp_log(LOG_WARNING, "Can't identify group (%s).\n", grouporgid);
    }

    return gid;
}
#endif /* NETSNMP_FEATURE_REMOVE_STR_TO_GID */

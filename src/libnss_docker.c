/*
 * nss-docker - NSS plugin for looking up Docker containers
 *  Copyright (c) 2015 Piotr Roszatycki <dexter@debian.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

#include <errno.h>
#include <nss.h>
#include <netdb.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#define ALIGN(a) (((a+sizeof(void*)-1)/sizeof(void*))*sizeof(void*))

enum nss_status _nss_docker_gethostbyname3_r(
    const char *name, int af,
    struct hostent *result, char *buffer, size_t buflen, int *errnop,
    int *herrnop, int32_t *ttlp, char **canonp
) {
    size_t idx;
    char *aliases, *addr_ptr, *addr_list;

    struct in_addr addr;
    addr.s_addr = 127 + 1 * 256 + 2 * 256 * 256 + 3 * 256 * 256 * 256; // 127.1.2.3

    fprintf(stderr, "_nss_docker_gethostbyname3_r(name=\"%s\", af=%d)\n", name, af);

    if (af != AF_INET) {
        *errnop = EAFNOSUPPORT;
        *herrnop = NO_DATA;
        return NSS_STATUS_UNAVAIL;
    }

    result->h_name = buffer;
    strcpy(result->h_name, name);

    idx = ALIGN(strlen(name) + 1);

    aliases = buffer + idx;
    *(char **) aliases = NULL;
    idx += sizeof(char*);

    result->h_aliases = (char **) aliases;

    result->h_addrtype = AF_INET;
    result->h_length = sizeof(struct in_addr);

    addr_ptr = buffer + idx;
    memcpy(addr_ptr, &addr, result->h_length);
    idx += ALIGN(result->h_length);

    addr_list = buffer + idx;
    ((char **) addr_list)[0] = addr_ptr;
    ((char **) addr_list)[1] = NULL;

    result->h_addr_list = (char **) addr_list;

    return NSS_STATUS_SUCCESS;
}

enum nss_status _nss_docker_gethostbyname2_r(
    const char *name, int af,
    struct hostent *result, char *buffer, size_t buflen, int *errnop,
    int *herrnop
) {
    return _nss_docker_gethostbyname3_r(
        name, af, result, buffer, buflen,
        errnop, herrnop, NULL, NULL
    );
}

enum nss_status _nss_docker_gethostbyname_r(
    const char *name,
    struct hostent *result, char *buffer, size_t buflen, int *errnop,
    int *herrnop
) {
    int af = AF_UNSPEC;

    return _nss_docker_gethostbyname3_r(
        name, af, result, buffer, buflen,
        errnop, herrnop, NULL, NULL
    );
}

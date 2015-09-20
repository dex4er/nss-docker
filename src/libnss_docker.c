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

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <nss.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#ifndef DEGUB
#define DEBUG 0
#endif

#ifndef DOCKER_SOCKET
#define DOCKER_SOCKET "/var/run/docker.sock"
#endif

#ifndef DOCKER_API_VERSION
#define DOCKER_API_VERSION "1.12"
#endif

#ifndef DOMAIN_SUFFIX
#define DOMAIN_SUFFIX ".docker"
#endif

#define ALIGN(a) (((a+sizeof(void*)-1)/sizeof(void*))*sizeof(void*))

#ifndef SUN_LEN
#define SUN_LEN(su) (sizeof(*(su)) - sizeof((su)->sun_path) + strlen((su)->sun_path))
#endif

#define DOCKER_API_REQUEST "GET /v" DOCKER_API_VERSION "/containers/%.32s/json HTTP/1.0\015\012\015\012"
#define HTTP_404 "HTTP/1.0 404"
#define FIND_IPADDRESS ",\"IPAddress\":\""


enum nss_status _nss_docker_gethostbyname3_r(
    const char *name, int af,
    struct hostent *result, char *buffer, size_t buflen, int *errnop,
    int *herrnop, int32_t *ttlp, char **canonp
) {
    size_t idx, recvlen, hostnamelen, ipaddresslen;
    char *aliases, *addr_ptr, *addr_list, *hostname_suffix, *begin_ipaddress, *end_ipaddress;
    char hostname[256];
    char ipaddress[16];
    struct in_addr addr;
    int sockfd, servlen;
    struct sockaddr_un serv_addr;
    char buffer_send[sizeof(DOCKER_API_REQUEST)+32], buffer_recv[10240];

    if (DEBUG) fprintf(stderr, "_nss_docker_gethostbyname3_r(name=\"%s\", af=%d)\n", name, af);

    if (af != AF_INET) {
        goto return_unavail_afnosupport;
    }

    hostnamelen = strlen(name);

    if (hostnamelen == 0) {
        goto return_notfound;
    }

    if (hostnamelen > 255) {
        hostnamelen = 255;
    }

    strncpy(hostname, name, sizeof(hostname));
    hostname[hostnamelen] = '\0';

    if ((hostname_suffix = strstr(hostname, DOMAIN_SUFFIX)) == NULL) {
        goto return_notfound;
    }

    if (hostname_suffix[sizeof(DOMAIN_SUFFIX) - 1] != '\0') {
        goto return_notfound;
    }

    *hostname_suffix = '\0';

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, DOCKER_SOCKET);
    servlen = SUN_LEN(&serv_addr);

    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        goto return_unavail_errno;
    }

    if (connect(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0) {
        goto return_unavail_errno;
    }

    snprintf(buffer_send, sizeof(buffer_send), DOCKER_API_REQUEST, hostname);

    if (write(sockfd, buffer_send, strlen(buffer_send)) < 0) {
        goto return_unavail_errno;
    }

    if ((recvlen = read(sockfd, buffer_recv, sizeof(buffer_recv)-1)) <= 0) {
        goto return_unavail_errno;
    }

    buffer_recv[recvlen] = '\0';

    if (DEBUG) fwrite(buffer_recv, recvlen, 1, stderr);

    if (strncmp(buffer_recv, HTTP_404, sizeof(HTTP_404)-1) == 0) {
        goto return_notfound;
    }

    if ((begin_ipaddress = strstr(buffer_recv, FIND_IPADDRESS)) == NULL) {
        goto return_unavail_badmsg;
    }

    begin_ipaddress += sizeof(FIND_IPADDRESS) - 1;

    if (*begin_ipaddress == '"') {
        goto return_notfound;
    }

    if ((end_ipaddress = index(begin_ipaddress, '"')) == NULL) {
        goto return_unavail_badmsg;
    }

    if ((ipaddresslen = end_ipaddress - begin_ipaddress) > 15) {
        goto return_unavail_badmsg;
    }

    if (ipaddresslen == 0) {
        goto return_notfound;
    }

    strncpy(ipaddress, begin_ipaddress, ipaddresslen);
    ipaddress[ipaddresslen] = '\0';

    if (DEBUG) fprintf(stderr, "ipaddress=\"%s\"\n", ipaddress);

    if (! inet_aton(ipaddress, &addr)) {
        goto return_unavail_badmsg;
    }

    result->h_name = buffer;
    strcpy(result->h_name, hostname);

    idx = ALIGN(strlen(hostname) + 1);

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

return_unavail_afnosupport:
    *errnop = EAFNOSUPPORT;
    *herrnop = NO_DATA;
    return NSS_STATUS_UNAVAIL;

return_unavail_badmsg:
    *errnop = EBADMSG;
    *herrnop = NO_DATA;
    return NSS_STATUS_UNAVAIL;

return_unavail_errno:
    *errnop = errno;
    *herrnop = NO_DATA;
    return NSS_STATUS_UNAVAIL;

return_notfound:
    *errnop = ENOENT;
    *herrnop = HOST_NOT_FOUND;
    return NSS_STATUS_NOTFOUND;
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

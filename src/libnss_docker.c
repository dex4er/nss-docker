/*
 * nss-docker - NSS plugin for looking up Docker containers
 *
 * Copyright (c) 2015-2016 Piotr Roszatycki <dexter@debian.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

#include <config.h>

#define _BSD_SOURCE
#define _DEFAULT_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>
#include <nss.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef DEBUG
#define DEBUG 0
#endif

/* defined in config.h */
/* #define DOCKER_SOCKET "/var/run/docker.sock" */
/* #define DOCKER_API_VERSION "1.21" */
/* #define DOCKER_DOMAIN_SUFFIX ".docker" */

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif

#define ALIGN(a) (((a+sizeof(void*)-1)/sizeof(void*))*sizeof(void*))

#ifndef SUN_LEN
#define SUN_LEN(su) (sizeof(*(su)) - sizeof((su)->sun_path) + strlen((su)->sun_path))
#endif

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define DOCKER_API_REQUEST "GET /v" DOCKER_API_VERSION "/containers/%." STR(HOST_NAME_MAX) "s/json HTTP/1.0\015\012\015\012"
#define HTTP_404 "HTTP/1.0 404"
#define FIND_NETWORKS ",\"Networks\":{"
#define FIND_IPADDRESS ",\"IPAddress\":\""


enum nss_status _nss_docker_gethostbyname3_r(
    const char *name, int af,
    struct hostent *result, char *buffer, size_t buflen, int *errnop,
    int *herrnop, int32_t *ttlp, char **canonp
) {
    /* full name length */
    size_t name_len;

    /* hostname is name without domain suffix */
    char hostname[256];

    /* docker domain suffix starts here */
    char *hostname_suffix_ptr;

    /* address of Docker API server */
    struct sockaddr_un docker_api_addr;

    /* length of docker_api_addr structure */
    socklen_t docker_api_addr_len;

    /* calculated buffer size */
    size_t buffer_size;

    /* offset for buffer ptr */
    size_t buffer_offset;

    /* socket descriptor */
    int sockfd;

    /* request message buffer */
    char req_message_buffer[sizeof(DOCKER_API_REQUEST) + HOST_NAME_MAX];

    /* request message size */
    size_t req_message_len;

    /* response message buffer */
    char res_message_buffer[20480];

    /* response message size */
    size_t res_message_len;

    /* response partial read size */
    ssize_t bytes_read;

    /* remaining bytes in the response buffer */
    size_t remaining;

    /* response message buffer ptr */
    char *partial_res_message_buffer;

    /* Buffer for IPAddress string value */
    char ipaddress_str[16];

    /* Pointer for begin of Networks value */
    char *begin_networks;

    /* Pointers for begin and end of IPAddress value */
    char *begin_ipaddress, *end_ipaddress;

    /* Length of IPAddress value */
    size_t ipaddress_len;

    /* IPAddress as in_addr */
    struct in_addr ipaddress_addr;

    /* List of aliases in hostent */
    char *aliases;

    /* First address in hostent */
    char *addr_ptr;

    /* List of addresses in hostent */
    char *addr_list;

    if (DEBUG) fprintf(stderr, "_nss_docker_gethostbyname3_r(name=\"%s\", af=%d)\n", name, af);

    /* Handle only IPv4 */
    if (af != AF_INET) {
        *errnop = EAFNOSUPPORT;
        goto return_unavail;
    }

    /* Basic assertions for host name */
    name_len = strlen(name);

    if (name_len == 0) {
        *errnop = EADDRNOTAVAIL;
        goto return_unavail;
    }

    if (name_len > 255) {
        name_len = 255;
    }

    strncpy(hostname, name, sizeof(hostname));
    hostname[name_len] = '\0';

    /* Handle only .docker domain */
    if ((hostname_suffix_ptr = strstr(hostname, DOCKER_DOMAIN_SUFFIX)) == NULL) {
        *errnop = EADDRNOTAVAIL;
        goto return_unavail;
    }

    if (hostname_suffix_ptr[sizeof(DOCKER_DOMAIN_SUFFIX) - 1] != '\0') {
        *errnop = EADDRNOTAVAIL;
        goto return_unavail;
    }

    *hostname_suffix_ptr = '\0';

    /* Connect to Docker API socket */
    memset((char *) &docker_api_addr, 0, sizeof(docker_api_addr));
    docker_api_addr.sun_family = AF_UNIX;
    strcpy(docker_api_addr.sun_path, DOCKER_SOCKET);
    docker_api_addr_len = SUN_LEN(&docker_api_addr);

    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        *errnop = errno;
        goto return_unavail;
    }

    if (connect(sockfd, (struct sockaddr *) &docker_api_addr, docker_api_addr_len) < 0) {
        *errnop = errno;
        goto return_unavail;
    }

    /* Prepare request message */
    req_message_len = snprintf(req_message_buffer, sizeof(req_message_buffer) - 1, DOCKER_API_REQUEST, hostname);

    if (req_message_len == sizeof(req_message_buffer) - 1) {
        req_message_buffer[req_message_len] = '\0';
    }

    /* Send request */
    if (write(sockfd, req_message_buffer, strlen(req_message_buffer)) < 0) {
        close(sockfd);
        *errnop = errno;
        goto return_unavail;
    }

    /* Receive response */
    res_message_len = 0;
    partial_res_message_buffer = res_message_buffer;
    remaining = sizeof(res_message_buffer) - 1;
    while((bytes_read = read(sockfd, partial_res_message_buffer, remaining)) > 0) {
      res_message_len += bytes_read;
      partial_res_message_buffer += bytes_read;
      remaining -= bytes_read;
    }

    close(sockfd);

    if (bytes_read < 0) {
        *errnop = errno;
        goto return_unavail;
    }

    if (res_message_len == 0) {
      goto return_unavail;
    }

    res_message_buffer[res_message_len] = '\0';

    if (DEBUG) fwrite(res_message_buffer, res_message_len, 1, stderr);

    /* Handle HTTP 404 Not Found */
    if (strncmp(res_message_buffer, HTTP_404, sizeof(HTTP_404) - 1) == 0) {
        goto return_notfound;
    }

    /* Check if there is Networks key */
    if ((begin_networks = strstr(res_message_buffer, FIND_NETWORKS)) == NULL) {
        /* If not, will search from the beginning */
        begin_networks = res_message_buffer;
    }

    /* Check if there is IPAddress key */
    if ((begin_ipaddress = strstr(begin_networks, FIND_IPADDRESS)) == NULL) {
        *errnop = EBADMSG;
        goto return_unavail;
    }

    /* Check if it looks like IPAddress value */
    begin_ipaddress += sizeof(FIND_IPADDRESS) - 1;

    if (*begin_ipaddress == '"') {
        goto return_notfound;
    }

    if ((end_ipaddress = strchr(begin_ipaddress, '"')) == NULL) {
        *errnop = EBADMSG;
        goto return_unavail;
    }

    if ((ipaddress_len = end_ipaddress - begin_ipaddress) > 15) {
        *errnop = EBADMSG;
        goto return_unavail;
    }

    if (ipaddress_len == 0) {
        goto return_notfound;
    }

    strncpy(ipaddress_str, begin_ipaddress, ipaddress_len);
    ipaddress_str[ipaddress_len] = '\0';

    if (DEBUG) fprintf(stderr, "ipaddress_str=\"%s\"\n", ipaddress_str);

    /* Convert string to in_addr */
    if (! inet_aton(ipaddress_str, &ipaddress_addr)) {
        *errnop = EBADMSG;
        goto return_unavail;
    }

    /* Prepare hostent result */
    result->h_name = buffer;

    buffer_size = ALIGN(strlen(name) + 1) + sizeof(char*) + ALIGN(sizeof(struct in_addr));

    if (buflen < buffer_size) {
        *errnop = ENOMEM;
        goto return_tryagain;
    }

    strcpy(result->h_name, name);

    buffer_offset = ALIGN(strlen(name) + 1);

    aliases = buffer + buffer_offset;
    *(char **) aliases = NULL;
    buffer_offset += sizeof(char*);

    result->h_aliases = (char **) aliases;

    result->h_addrtype = AF_INET;
    result->h_length = sizeof(struct in_addr);

    addr_ptr = buffer + buffer_offset;
    memcpy(addr_ptr, &ipaddress_addr, result->h_length);
    buffer_offset += ALIGN(result->h_length);
    assert(buffer_offset == buffer_size);

    addr_list = buffer + buffer_offset;
    ((char **) addr_list)[0] = addr_ptr;
    ((char **) addr_list)[1] = NULL;

    result->h_addr_list = (char **) addr_list;

    return NSS_STATUS_SUCCESS;

return_unavail:
    *herrnop = NO_DATA;
    return NSS_STATUS_UNAVAIL;

return_tryagain:
    *herrnop = NO_RECOVERY;
    return NSS_STATUS_TRYAGAIN;

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

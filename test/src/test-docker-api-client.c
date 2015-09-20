#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef SUN_LEN
#define SUN_LEN(su) (sizeof(*(su)) - sizeof((su)->sun_path) + strlen((su)->sun_path))
#endif

#define DOCKER_API_REQUEST "GET /v%s/containers/%s/json HTTP/1.0\015\012\015\012"


int main(int argc, char *argv[]) {
    int sockfd, servlen, n;
    struct sockaddr_un serv_addr;
    char buffer[10240];

    if (argc != 4) {
        fprintf(stderr, "Usage: %s docker_socket api_version container_id\n", argv[0]);
        exit(2);
    }

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, argv[1]);
    servlen = SUN_LEN(&serv_addr);

    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    if (connect(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0) {
        perror("connect");
        exit(1);
    }

    memset(buffer, 0, 10240);
    snprintf(buffer, 10240, DOCKER_API_REQUEST, argv[2], argv[3]);
    if (write(sockfd, buffer, strlen(buffer)) < 0) {
        perror("write(newsockfd)");
        exit(1);
    }

    n = read(sockfd, buffer, 10240);
    if (write(1, buffer, n) < 0) {
        perror("write(newsockfd)");
        exit(1);
    }

    return 0;
}

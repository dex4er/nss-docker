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

#define DOCKER_RESPONSE_SUCCESS "HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nDate: Sun, 20 Sep 2015 19:53:31 GMT\r\n\r\n{\"Id\":\"12b2dd585ecbdf168ad8a3645744445afe58794af7fdf2ab2b1bb97b6af3fc4a\",\"Created\":\"2015-09-20T11:24:42.329765535Z\",\"Path\":\"/sbin/init\",\"Args\":[],\"State\":{\"Running\":true,\"Paused\":false,\"Restarting\":false,\"OOMKilled\":false,\"Dead\":false,\"Pid\":28145,\"ExitCode\":0,\"Error\":\"\",\"StartedAt\":\"2015-09-20T11:24:42.850759517Z\",\"FinishedAt\":\"0001-01-01T00:00:00Z\"},\"Image\":\"55c1102409b9642a20b5f1d1d894a8419aaaf539e867005d36ac4ffd59abed012a2\",\"NetworkSettings\":{\"Bridge\":\"\",\"EndpointID\":\"4fb52aafa1874afea14712e1a71ab85ffec7d508f5f5b2b302bbf54a1e398853\",\"Gateway\":\"172.17.42.1\",\"GlobalIPv6Address\":\"\",\"GlobalIPv6PrefixLen\":0,\"HairpinMode\":false,\"IPAddress\":\"%s\",\"IPPrefixLen\":16,\"IPv6Gateway\":\"\",\"LinkLocalIPv6Address\":\"\",\"LinkLocalIPv6PrefixLen\":0,\"MacAddress\":\"02:42:ac:11:00:02\",\"NetworkID\":\"ea191dc39711f748a4b26d03abdb98817135b65a53d3c2beba6586d9ffceaf31\",\"PortMapping\":null,\"Ports\":{},\"SandboxKey\":\"/var/run/docker/netns/12b2dd585ecb\",\"SecondaryIPAddresses\":null,\"SecondaryIPv6Addresses\":null},\"ResolvConfPath\":\"/var/lib/docker/containers/12b2dd585ecbdf168ad8a3645744445afe58794af7fdf2ab2b1bb97b6af3fc4a/resolv.conf\",\"HostnamePath\":\"/var/lib/docker/containers/12b2dd585ecbdf168ad8a3645744445afe58794af7fdf2ab2b1bb97b6af3fc4a/hostname\",\"HostsPath\":\"/var/lib/docker/containers/12b2dd585ecbdf168ad8a3645744445afe58794af7fdf2ab2b1bb97b6af3fc4a/hosts\",\"LogPath\":\"/var/lib/docker/containers/12b2dd585ecbdf168ad8a3645744445afe58794af7fdf2ab2b1bb97b6af3fc4a/12b2dd585ecbdf168ad8a3645744445afe58794af7fdf2ab2b1bb97b6af3fc4a-json.log\",\"Name\":\"/%s\",\"RestartCount\":0,\"Driver\":\"aufs\",\"ExecDriver\":\"native-0.2\",\"MountLabel\":\"\",\"ProcessLabel\":\"\",\"Volumes\":{},\"VolumesRW\":{},\"AppArmorProfile\":\"\",\"ExecIDs\":null,\"HostConfig\":{\"Binds\":null,\"ContainerIDFile\":\"\",\"LxcConf\":[],\"Memory\":0,\"MemorySwap\":0,\"CpuShares\":0,\"CpuPeriod\":0,\"CpusetCpus\":\"\",\"CpusetMems\":\"\",\"CpuQuota\":0,\"BlkioWeight\":0,\"OomKillDisable\":false,\"Privileged\":false,\"PortBindings\":{},\"Links\":null,\"PublishAllPorts\":false,\"Dns\":null,\"DnsSearch\":null,\"ExtraHosts\":null,\"VolumesFrom\":null,\"Devices\":[],\"NetworkMode\":\"bridge\",\"IpcMode\":\"\",\"PidMode\":\"\",\"UTSMode\":\"\",\"CapAdd\":null,\"CapDrop\":null,\"RestartPolicy\":{\"Name\":\"no\",\"MaximumRetryCount\":0},\"SecurityOpt\":null,\"ReadonlyRootfs\":false,\"Ulimits\":null,\"LogConfig\":{\"Type\":\"json-file\",\"Config\":{}},\"CgroupParent\":\"\"},\"Config\":{\"Hostname\":\"12b2dd585ecb\",\"Domainname\":\"\",\"User\":\"\",\"AttachStdin\":false,\"AttachStdout\":true,\"AttachStderr\":true,\"PortSpecs\":null,\"ExposedPorts\":null,\"Tty\":false,\"OpenStdin\":false,\"StdinOnce\":false,\"Env\":[\"PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin\"],\"Cmd\":null,\"Image\":\"hello-world\",\"Volumes\":null,\"VolumeDriver\":\"\",\"WorkingDir\":\"\",\"Entrypoint\":[\"/hello\"],\"NetworkDisabled\":false,\"MacAddress\":\"\",\"OnBuild\":null,\"Labels\":{},\"Memory\":0,\"MemorySwap\":0,\"CpuShares\":0,\"Cpuset\":\"\"}}\n"
#define DOCKER_RESPONSE_NOTFOUND "HTTP/1.0 404 Not Found\r\nContent-Type: text/plain; charset=utf-8\r\nDate: Sun, 20 Sep 2015 19:58:43 GMT\r\n\r\nno such id: %s\n"


int main(int argc, char *argv[]) {
    int sockfd, newsockfd, servlen, n, i, how_many;
    socklen_t clilen;
    struct sockaddr_un cli_addr, serv_addr, name_addr;
    socklen_t namelen;
    char buffer[10240];
    char *begin_container, *end_container;
    size_t containerlen;

    if (argc != 4 && argc != 5) {
        fprintf(stderr, "Usage: %s how_many docker_socket container_id ip_address\n", argv[0]);
        exit(2);
    }

    how_many = atoi(argv[1]);

    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, argv[2]);
    servlen = SUN_LEN(&serv_addr);
    if (bind(sockfd, (struct sockaddr *)&serv_addr, servlen) < 0) {
        perror("bind");
        exit(1);
    }

    namelen = SUN_LEN(&name_addr);
    if (getsockname(sockfd, (struct sockaddr*)&name_addr, &namelen) < 0) {
        perror("getsockname");
        exit(1);
    }

    listen(sockfd, 1);
    clilen = sizeof(cli_addr);

    for (i = 0; how_many == 0 || i < how_many; i++) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            perror("accept");
            exit(1);
        }

        n = read(newsockfd, buffer, 10240);
        if (write(1, buffer, n) < 0) {
            perror("write(1)");
            exit(1);
        }

        if ((begin_container = strstr(buffer, "/containers/")) == NULL) {
            exit(1);
        }

        begin_container += sizeof("/containers/") - 1;

        if ((end_container = index(begin_container, '/')) == NULL) {
            exit(1);
        }

        *end_container = '\0';

        if ((containerlen = end_container - begin_container) == 0) {
            sprintf(buffer, DOCKER_RESPONSE_NOTFOUND, "CONTAINER");
        } else if (strncmp(begin_container, argv[3], containerlen) != 0) {
            sprintf(buffer, DOCKER_RESPONSE_NOTFOUND, begin_container);
        } else {
            sprintf(buffer, DOCKER_RESPONSE_SUCCESS, argv[4], argv[3]);
        }

        if (write(newsockfd, buffer, strlen(buffer)) < 0) {
            perror("write(newsockfd)");
            exit(1);
        }

        close(newsockfd);
    }

    return 0;
}

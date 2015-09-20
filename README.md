[![Build Status](https://travis-ci.org/dex4er/nss-docker.png?branch=master)](https://travis-ci.org/dex4er/nss-docker)

This is NSS module for finding Docker containers.

Install it and add this library to your `/etc/nsswitch.conf` file:

```
hosts: files docker [NOTFOUND=return] dns
```

The container ID is searched in virtual `.docker` domain:

```
$ docker run -d --name test dex4er/alpine-init
a548fba14c314e77a1630580f6536e09002d84e5610b85a08c622fa451a4f893

$ getent hosts test.docker
172.17.0.4      test

$ ping test.docker
PING localhost (172.17.0.4) 56(84) bytes of data.
64 bytes from test.docker (172.17.0.4): icmp_seq=1 ttl=64 time=0.171 ms
```

[![Build Status](https://travis-ci.org/dex4er/nss-docker.png?branch=master)](https://travis-ci.org/dex4er/nss-docker)

This is a plugin for the GNU Name Service Switch (NSS) functionality of
the GNU C Library (glibc) providing mechanism for finding Docker
containers by theirs IDs.

The container names are searched in virtual domain name ".docker".

Install it and add this library to your `/etc/nsswitch.conf` file:

```
hosts: files docker [NOTFOUND=return] dns
```

Then try:

```
$ docker run -d --name test dex4er/alpine-init
a548fba14c314e77a1630580f6536e09002d84e5610b85a08c622fa451a4f893

$ getent hosts test.docker
172.17.0.4      test.docker

$ ping test.docker
PING test.docker (172.17.0.4) 56(84) bytes of data.
64 bytes from 172.17.0.4: icmp_seq=1 ttl=64 time=0.171 ms
```

This module does not require any additional libraries beside GNU C Library.

## Installation

From Git repository:

```
$ git clone https://github.com/dex4er/nss-docker
$ ./autogen.sh
$ ./configure --prefix=/usr
$ make
$ sudo make install
```

## Testing

The tests requires root priviliges (for real chroot because `/etc/nsswitch.conf` can't be changed to another file)

It is suggested to compile necessary files (ie.: mock server) as standard user and run tests as root user.

```
$ make -C test check-src
$ sudo make test
```

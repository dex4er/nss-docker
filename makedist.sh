#!/bin/bash

./autogen.sh
./configure

( cd man && make nss-docker.1 )

make dist

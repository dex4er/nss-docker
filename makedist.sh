#!/bin/bash

./autogen.sh
./configure

( cd man && make nss-docker.8 )

make dist

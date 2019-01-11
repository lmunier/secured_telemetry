#!/usr/bin/env bash

# Linker opts should be blank for OS X, FreeBSD and OpenSolaris
#LINKER_OPTIONS=""

# On Linux, we must link with realtime and thread libraries
LINKER_OPTIONS="-lrt -lpthread"

g++ -Wall -c -o utils.o utils.cpp
g++ -g3 -ggdb -O0 -DDEBUG -I/usr/include/cryptopp -c -o encrypt_rc5.o encrypt_rc5.cpp -lcryptopp -lpthread -std=c++11
g++ -g3 -ggdb -O0 -DDEBUG -I/usr/include/cryptopp utils.o encrypt_rc5.o -o receive_encrypt.x receive_encrypt.cpp -L. $LINKER_OPTIONS -lcryptopp -lpthread -std=c++11

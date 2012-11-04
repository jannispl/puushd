#!/bin/bash

gcc *.c -c

g++ -std=c++0x *.cpp ./MPFDParser/*.cpp ./pages/*.cpp sqlite3.o mongoose.o md5.o -o puushd -ldl -lpthread

rm *.o

#!/bin/sh

g++ -g lzf_* libuinput.c keyboardState.c compression.cpp -lrt

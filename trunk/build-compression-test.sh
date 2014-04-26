#!/bin/bash

g++  lzf*.h lzf*.c compressionTest.c -lrt -L/usr/lib -ljpeg -o compressiontest.out

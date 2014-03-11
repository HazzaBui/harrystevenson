#ifndef _KEYBOARDSTATE_H
#define _KEYBOARDSTATE_H

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <linux/input.h>
#include <linux/uinput.h>
#include "libuinput.h"

//function signatures
int initialiseKeyboard();
int updateState();
int changeState(char* stream, short length);
int uninitialise();

//vars
static struct uinput_tkn * keybd;
static int numOfKeys = 33;

#endif

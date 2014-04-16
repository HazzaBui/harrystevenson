#include "keyboardState.h"

//structs
static struct uinput_user_dev uudev = {
		name: "libuinput-keyboard-demo",
		id: {
				bustype: BUS_USB,
				vendor: 0x1,
				product: 0x1,
				version: 1,
		},
};

static struct input_events_custom valid_events[] = {
	{UI_SET_KEYBIT,KEY_ENTER, 10},
	{UI_SET_KEYBIT,KEY_BACKSPACE, 8},
	{UI_SET_KEYBIT,KEY_LEFTSHIFT, 16},
	{UI_SET_KEYBIT,KEY_LEFTCTRL, 17},
	{UI_SET_KEYBIT,KEY_SPACE, 32},
	{UI_SET_KEYBIT,KEY_TAB, 9},
	{UI_SET_KEYBIT,KEY_SEMICOLON, 59},
	{UI_SET_KEYBIT,KEY_ESC, 27},
	{UI_SET_KEYBIT,KEY_A, 65},
	{UI_SET_KEYBIT,KEY_B, 66},
	{UI_SET_KEYBIT,KEY_C, 67},
	{UI_SET_KEYBIT,KEY_D, 68},
	{UI_SET_KEYBIT,KEY_E, 69},
	{UI_SET_KEYBIT,KEY_F, 70},
	{UI_SET_KEYBIT,KEY_G, 71},
	{UI_SET_KEYBIT,KEY_H, 72},
	{UI_SET_KEYBIT,KEY_I, 73},
	{UI_SET_KEYBIT,KEY_J, 74},
	{UI_SET_KEYBIT,KEY_K, 75},
	{UI_SET_KEYBIT,KEY_L, 76},
	{UI_SET_KEYBIT,KEY_M, 77},
	{UI_SET_KEYBIT,KEY_N, 78},
	{UI_SET_KEYBIT,KEY_O, 79},
	{UI_SET_KEYBIT,KEY_P, 80},
	{UI_SET_KEYBIT,KEY_Q, 81},
	{UI_SET_KEYBIT,KEY_R, 82},
	{UI_SET_KEYBIT,KEY_S, 83},
	{UI_SET_KEYBIT,KEY_T, 84},
	{UI_SET_KEYBIT,KEY_U, 85},
	{UI_SET_KEYBIT,KEY_V, 86},
	{UI_SET_KEYBIT,KEY_W, 87},
	{UI_SET_KEYBIT,KEY_X, 88},
	{UI_SET_KEYBIT,KEY_Y, 89},
	{UI_SET_KEYBIT,KEY_Z, 90},
	{UI_SET_KEYBIT,KEY_1, 49},
	{UI_SET_KEYBIT,KEY_2, 50},
	{UI_SET_KEYBIT,KEY_3, 51},
	{UI_SET_KEYBIT,KEY_4, 52},
	{UI_SET_KEYBIT,KEY_5, 53},
	{UI_SET_KEYBIT,KEY_6, 54},
	{UI_SET_KEYBIT,KEY_7, 55},
	{UI_SET_KEYBIT,KEY_8, 56},
	{UI_SET_KEYBIT,KEY_9, 57},
	{UI_SET_KEYBIT,KEY_0, 48},
	{UI_SET_KEYBIT,KEY_UP, 38},
	{UI_SET_KEYBIT,KEY_DOWN, 40},
	{UI_SET_KEYBIT,KEY_LEFT, 37},
	{UI_SET_KEYBIT,KEY_RIGHT, 39},


};



int initialiseKeyboard()
{

	fprintf(stdout,"\n\nStart");
for(int i = 0; i < 30; i++)
{
	fprintf(stdout,"\n%d - %d", valid_events[i].code, valid_events[i].asciicode);
}

	fprintf(stdout,"\nEND\n");
	int err;

	keybd = uinput_open(NULL);
	if(!keybd)
	{
		fprintf(stdout, "\nFailed to create keyboard device ");
		return -1;
	}
	
	err = uinput_enable_event(keybd, EV_KEY);
	if(err)
	{
		fprintf(stdout, "\nFailed to enable keyboard events ");
		return -1;
	}

	err = uinput_set_valid_events(keybd, valid_events, numOfKeys);
	if(err)
	{
		fprintf(stdout, "\nFailed to set valid keys: %d ", err);
		return -1;
	}

	err = uinput_create_new_device(keybd, &uudev);
	if(err)
	{
		fprintf(stdout, "\nFailed to register keyboard ", err);
		return -1;
	}
}

int changeState(char* stream, short length)
{
	input_event tevent = {{0, 0}, EV_KEY, KEY_Q, 1};
fprintf(stdout, "\nTranslating input:");
	for(int i = 0; i < numOfKeys; i++)
	{
		for(int j = 0; j < length; j++)
		{
			if(stream[j] == valid_events[i].asciicode)
			{
				tevent.code = valid_events[i].code;					
				tevent.value = (int)stream[j+1];
				uinput_send_event(keybd, &tevent);
				fprintf(stdout, "%d-%d:", valid_events[i].asciicode, valid_events[i].code); 
			}
			j++;
		}
	}
fprintf(stdout,"\ndone");
}


int uninitialise()
{
	fprintf(stdout, "\nClosing keyboard device");
	uinput_close(keybd);
}

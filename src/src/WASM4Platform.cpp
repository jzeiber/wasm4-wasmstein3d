#include "WASM4Platform.h"
//#include "Generated/Data_Audio.h"

WASM4Platform Platform;

void WASM4Platform::update()
{
	inputState = 0;

	if(*GAMEPAD1 & BUTTON_1)
	{
		inputState|=Input_Btn_B;
	}
	if(*GAMEPAD1 & BUTTON_2)
	{
		inputState|=Input_Btn_A;
	}
	if(*GAMEPAD1 & BUTTON_UP)
	{
		inputState|=Input_Dpad_Up;
	}
	if(*GAMEPAD1 & BUTTON_LEFT)
	{
		inputState|=Input_Dpad_Left;
	}
	if(*GAMEPAD1 & BUTTON_RIGHT)
	{
		inputState|=Input_Dpad_Right;
	}
	if(*GAMEPAD1 & BUTTON_DOWN)
	{
		inputState|=Input_Dpad_Down;
	}

}

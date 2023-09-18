/*
  NHD_lib.h - Library for controlling a NewHaven Display.
  I tested this library with the NHD-0420D3Z-FL-GBW-V3_revB
  Does not work with more than one screen
  Created by Vincent Goulet, August 19, 2023.
*/

#ifndef NHD_lib_h
#define NHD_lib_h

#include <Arduino.h>

enum Direction {
	left = 0, right = 1
};

class NHD_lib
{
	public:
		NHD_lib(bool displayState, bool underlineState, bool blinkState, int contrast, int brightness);
		void begin(unsigned int speed);
		void DisplayState(bool state);
		void SetCursor(int lign, int column);
		void CursorHome();
		void UnderlineCursor(bool state);
		void MoveCursor(Direction dir, int nbr);
		void BlinkCursor(bool state);
		void Backspace(int nbr);
		void ClearScreen();
		void SetContrast(int contrast);
		int Contrast();
		void SetBrightness(int bright);
		int Brightness();
		//void LoadCustomChar(int address, int B1. int B2, int B3, int B4, int B5, int B6, int B7, int B8);
		void MoveDisplay(Direction dir, int nbr);
		void DisplayFirmwareVersion();

	private:
		unsigned int _speed;
		bool _state_display;
		int _lign;
		int _column;
		int _pos;
		bool _state_underline;
		Direction _dir_move;
		int _nbr_move;
		bool _state_blink;
		int _nbr_backspace;
		int _contrast;
		int _bright;
		Direction _dir_display;
		int _nbr_display;
};

#endif
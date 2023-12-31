/*
  NHD_lib.cpp - Library for controlling a NewHaven Display.
  I tested this library with the NHD-0420D3Z-FL-GBW-V3_revB
  Does not work with more than one screen
  Created by Vincent Goulet, August 19, 2023.
*/

#include "NHD_lib.h"



NHD_lib::NHD_lib(bool displayState, bool underlineState, bool blinkState, int contrast, int brightness, HardwareSerial * serial){
  _state_display = displayState;
  _state_underline = underlineState;
  _state_blink = blinkState;
  _contrast = contrast;
  _bright = brightness;
  _serial = serial;
  //HardwareSerial SerialPort(2); // use UART2
  
}

void NHD_lib::begin(unsigned int speed){
	_speed = speed;
	
	_serial->begin(_speed, SERIAL_8N1, 16, 17);
  
	DisplayState(_state_display);
	UnderlineCursor(_state_underline);
	BlinkCursor(_state_blink);
	SetContrast(_contrast);
	SetBrightness(_bright);
	
	ClearScreen();
	DisplayFirmwareVersion();
	delay(2000);
	ClearScreen();
}

void NHD_lib::DisplayState(bool state){
  _state_display = state;
  if(_state_display == 1){
    _serial->write(0xFE); //Display ON
    _serial->write(0x41);
  } 
  else{
    _serial->write(0xFE); //Display OFF
    _serial->write(0x42);
  }
  delay(0.1);
}

void NHD_lib::SetCursor(int lign, int column){
  _lign = lign;
  _column = column;
  
  if(_column >= 20){
	_column = 20;
  } 
  else if(_column <= 1){
	_column = 1;
  }
   
  if(_lign <= 1){
	_pos = 0x00 + (_column - 1);
  }
  else if(_lign == 2){
	_pos = 0x40 + (_column - 1);
  }
  else if(_lign == 3){
	_pos = 0x14 + (_column - 1);
  }
  else if(_lign >= 4){
	_pos = 0x54 + (_column - 1);
  }
  _serial->write(0xFE);
  _serial->write(0x45);
  _serial->write(_pos);
  delay(0.1);
}

void NHD_lib::CursorHome(){
	_serial->write(0xFE);
	_serial->write(0x46); 
	delay(1.5);
}

void NHD_lib::UnderlineCursor(bool state){
    _state_underline = state;
    if(_state_underline == 1){
        _serial->write(0xFE); //Underline Cursor ON
        _serial->write(0x47);
    } 
    else{
        _serial->write(0xFE); //Underline Cursor OFF
		_serial->write(0x48);
	}
	delay(1.5);
}

void NHD_lib::MoveCursor(Direction dir, int nbr){
	_dir_move = dir;
	_nbr_move = nbr;
	
	for(int move = 0; move < _nbr_move; move++){
		if(_dir_move == left){
			_serial->write(0xFE); //Move Left
			_serial->write(0x49);
		}
		else if(_dir_move == right){
			_serial->write(0xFE); //Move Right
			_serial->write(0x4A);
		}
		delay(0.1);
	}
}

void NHD_lib::BlinkCursor(bool state){
	_state_blink = state;
	if(_state_blink == 1){
		_serial->write(0xFE); //Blinking Cursor ON
		_serial->write(0x4B);
	} 
	else{
		_serial->write(0xFE); //Blinking Cursor OFF
		_serial->write(0x4C);
	}
	delay(0.1);
}

void NHD_lib::Backspace(int nbr){
	_nbr_backspace = nbr;
	for(int back = 0; back < _nbr_backspace; back++){
		_serial->write(0xFE);
		_serial->write(0x4E); 
		delay(0.1);
	}
}

void NHD_lib::ClearScreen(){
	_serial->write(0xFE);
    _serial->write(0x51); 
	delay(1.5);
}

void NHD_lib::SetContrast(int contrast){
  _contrast = contrast;
  if(_contrast >= 50){
    _contrast = 50;
  }
  else if(_contrast <= 1){
    _contrast = 1;
  }
  _serial->write(0xFE);
  _serial->write(0x52);
  _serial->write(_contrast);
  delay(0.5);
}

int NHD_lib::Contrast(){
	int tmp = _contrast;
	return tmp;
}

void NHD_lib::SetBrightness(int bright){
  _bright = bright;
  if(_bright >= 8){
    _bright = 8;
  }
  else if(_bright <= 1){
    _bright = 1;
  }
  _serial->write(0xFE);
  _serial->write(0x53);
  _serial->write(_bright);
  delay(0.1);
}

int NHD_lib::Brightness(){
	int tmp = _bright;
	return tmp;
}

/*void NHD_lib::LoadCustomChar(int address, int B1. int B2, int B3, int B4, int B5, int B6, int B7, int B8){
	
}*/

void NHD_lib::MoveDisplay(Direction dir, int nbr){
	_dir_display = dir;
	_nbr_display = nbr;
	
	for(int display = 0; display < _nbr_display; display++){
		if(_dir_display == left){
			_serial->write(0xFE); //Move Display Left
			_serial->write(0x55);
		}
		else if(_dir_display == right){
			_serial->write(0xFE); //Move Display Right
			_serial->write(0x56);
		}
		delay(0.1);
	}
}

void NHD_lib::DisplayFirmwareVersion(){
	_serial->write(0xFE);
    _serial->write(0x70);
	delay(4);
}
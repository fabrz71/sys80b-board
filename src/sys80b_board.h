/*
 Name:		sys80b_board.h
 Created:	15/03/2020 14:46:57
 Author:	Fabrizio
 Editor:	http://www.visualmicro.com
*/

#ifndef _sys80b_board_h
#define _sys80b_board_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <SD_t3.h>
#include <SD.h>
#include <SPI.h>
#include <LiquidCrystalFast.h>
#include <Bounce.h>
#include <EEPROM.h>

#include "Board_Sys80b.h"
//Board_Sys80b board;

//void setup();
//void loop();
void test(Board_Sys80b board);

#endif


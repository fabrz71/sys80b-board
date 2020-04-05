/* *** Gottileb System 80/B SIM PRB ***
* (SIMulation Pinball Replacement control Board)
* software for Teensy 3.2 board developed with Arduino IDE
* under GNU General Public Livence v3
* by Fabrizio Volpi (fabvolpi@gmail.com)
* ---
* ON-board 8x8 LED array programmer interface
* ---
* Functions for on-board LED array.
* (Static Class)
*/

#ifndef _Led8x8_h_
#define _Led8x8_h_

#include "Arduino.h"

class SPISettings;

class Led8x8 {
public:
	static const byte CMD_DECODE = 0x9;
	static const byte CMD_INTNSTY = 0xa;
	static const byte CMD_DIGITS = 0xb;
	static const byte CMD_SHUTDWN = 0xc;

	bool enabled;
	bool writeOptimization;

protected:
	SPISettings* _spis;
	int _ss_pin;
	byte* _row;

public:
	Led8x8(SPISettings* ss, int pin);
	bool setRowByte(byte row, byte data);
	bool setRow(byte row, byte data);
	void clear();
	void shutDownMode(bool b);
	void setIntensity(byte i);
	inline byte getRow(byte i) { return _row[i & 7]; }

protected:
	void _writeCmd(byte adr, byte data);
};

#endif

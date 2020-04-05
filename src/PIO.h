/* *** Gottileb System 80/B SIM PRB ***
* (SIMulation Pinball Replacement control Board)
* software for Teensy 3.2 board developed with Arduino IDE
* under GNU General Public Livence v3
* ---
* LOW LEVEL I/O
* ---
* Low level I/O functions for MCP24S17 GPIO expanders.
* MCP24S17 IC offers two 8-bit I/O ports and it's controlled
* by Teensy through SPI interface.
*/

#ifndef _Mcp_io_h_
#define _Mcp_io_h_

#include "Arduino.h"

//#include "ioPins.h"

class SPISettings;

class PIO {
public:
	static const byte IODIRA = 0x00;	// MCP23x17 I/O Direction Register
	static const byte IODIRB = 0x01;	// MCP23x17 I/O Direction Register
	static const byte IPOLA = 0x02;	// MCP23x17 Input Polarity Register
	static const byte IOCON = 0x0A;	// MCP23x17 Configuration Register
	static const byte GPPUA = 0x0C;	// MCP23x17 Weak Pull-Up Resistor Register
	static const byte GPPUB = 0x0D;	// MCP23x17 Weak Pull-Up Resistor Register
	static const byte GPIOA = 0x12;	// MCP23x17 GPIO Port A Register
	static const byte GPIOB = 0x13;	// MCP23x17 GPIO Port B Register

	static const byte ADDR_ENABLE = 0b00001000; // Configuration register for MCP23S17, the only thing we change is enabling hardware addressing
	static const byte OPCODEW = 0b01000000; // Opcode for MCP23S17 with LSB (bit0) set to write (0), address OR'd in later, bits 1-3
	static const byte OPCODER = 0b01000001; // Opcode for MCP23S17 with LSB (bit0) set to read (1), address OR'd in later, bits 1-3

protected:
	SPISettings* _spis;
	uint8_t _ss_pin;
	uint8_t _adr;

public:
	PIO(SPISettings* ss, uint8_t ss_pin, uint8_t address, uint16_t pinsConfig);
	uint16_t mcpRead();
	byte mcpReadPA();
	byte mcpReadPB();
	void mcpWrite(uint16_t value);
	void mcpWritePA(uint8_t value);
	void mcpWritePB(uint8_t value);

protected:
	void _byteWrite(uint8_t reg, uint8_t value);
	byte _byteRead(uint8_t reg);
	void _wordWrite(uint8_t reg, uint16_t value);
	void _pinDirection(uint16_t mode);
	void _SPIstart();
	void _SPIstop();
	inline void _pullupMode(uint16_t mode) { _wordWrite(GPPUA, mode); }
	inline void _inputInvert(uint16_t mode) { _wordWrite(IPOLA, mode); }
};

#endif

#include "Board_Sys80b.h"

#include <SPI.h>
#include <Bounce.h>

#define BOARD_SW_REV F("0.2003")

Board_Sys80b::Board_Sys80b() {	
	// pins setup
	pinMode(SLAM_PIN, INPUT_PULLUP);
	pinMode(NEXTB_PIN, INPUT_PULLUP);
	pinMode(ENTRB_PIN, INPUT_PULLUP);
	pinMode(BACKB_PIN, INPUT_PULLUP);
	pinMode(REV_D0_PIN, INPUT_PULLUP);
	pinMode(REV_D1_PIN, INPUT_PULLUP);
	pinMode(GPIO_SS_PIN, OUTPUT);
	pinMode(LG_SS_PIN, OUTPUT);
	pinMode(SD_SS_PIN, OUTPUT);
	pinMode(D_LD1_PIN, OUTPUT);
	pinMode(D_LD2_PIN, OUTPUT);
	pinMode(D_RES_PIN, OUTPUT);
	digitalWrite(GPIO_SS_PIN, HIGH);
	digitalWrite(LG_SS_PIN, HIGH);
	digitalWrite(SD_SS_PIN, HIGH);
	digitalWrite(D_LD1_PIN, LOW);
	digitalWrite(D_LD2_PIN, LOW);
	digitalWrite(D_RES_PIN, LOW);

	Serial.begin(SERIAL_BAUDRATE);
	_spiSettings = new SPISettings(SPI_CLK, MSBFIRST, SPI_MODE0);
	_poc = 0;
	_lcd = nullptr;
}

Board_Sys80b::~Board_Sys80b() {
	//delete lcd;
	delete ledGrid;
	delete nextButton;
	delete enterButton;
	delete backButton;
	delete _spiSettings;
	delete NVData;
}

bool Board_Sys80b::begin() {
	const char* hdr = "begin";
	msg.enableSerialOutput();
	msg.setPostDelay(0);
	msg.disableLCDOutput();
	msg.enableLogOutput();
	msg.normalOutputToLog = false;
	_revision = _readHwRev();
	msg.out(F("Starting board rev."));
	msg.outln(_revision);
	//delay(100);

	// SPI init
	msg.outln(F("- SPI init..."));
	SPI.begin();
	//delay(100);

	// SD init
	msg.outln(F("- SD init..."));
	if (!SD.begin(SD_SS_PIN)) {
		msg.err(F("can't init SD card!"), CLASS_NAME, hdr);
		return false;
	}
	//delay(100);

	// PIO init (Parallel Input/Output)
	msg.outln(F("- PIOs init..."));
	solenoids_IO = new PIO(_spiSettings, GPIO_SS_PIN, 0, 0x0000); // all 16 pins set as output
	lamps_IO = new PIO(_spiSettings, GPIO_SS_PIN, 1, 0x0000); // all 16 pins set as output
	switchMatrix_IO = new PIO(_spiSettings, GPIO_SS_PIN, 2, 0xff00); // 8 bits output + 8 bits input
	displayAndSound_IO = new PIO(_spiSettings, GPIO_SS_PIN, 3, 0x0000); // all 16 pins set as output
	outputReset(); // hardware immediate resets
	//delay(100);

	msg.outln(F("- Buttons init..."));
	nextButton = new Bounce(NEXTB_PIN, DEBOUNCE_T);
	enterButton = new Bounce(ENTRB_PIN, DEBOUNCE_T);
	backButton = new Bounce(BACKB_PIN, DEBOUNCE_T);
	//delay(100);

	msg.out(F("- NVRAM init... "));
	NVData = new NVData_class();
	NVData->immediateWriteMode = false;
	//NVData->begin(NVData->EEPROM_NVD, 128, SD_SS_PIN);
	if (_revision == 0) NVData->begin(NVData->SDFILE_NVD, 128, SD_SS_PIN);
	if (NVData->ready()) {
		msg.outln(F("ok."));
		msg.out(F("- Power On Counter "));
		msg.outln(_poc);
		_poc = incrementPowerOnCounter();
		msg.out(F("- New Power On Counter: "));
		msg.outln(_poc);
	}
	else {
		msg.warn(F("Troubles with NV Data initizlization: NV not available!"), CLASS_NAME, hdr);
	}
	//delay(100);

	msg.outln(F("- Led grid init..."));
	delay(50);
	ledGrid = new Led8x8(_spiSettings, LG_SS_PIN);
	delay(10);
	ledGrid->setIntensity(1);
	ledGrid->enabled = true;
	//ledGrid->clear();
	//ledGrid->setRow(0, 1);
	//delay(100);

#ifndef NO_LCD_MOUNTED
	// messages output initialization
	msg.outln(F("- LCD init... "));
	lcd = new LiquidCrystalFast(LCD_RS_PIN, LCD_EN_PIN,
		LCD_D4_PIN, LCD_D5_PIN, LCD_D6_PIN, LCD_D7_PIN);
	//Serial.printf("%d rows, %d cols\n", rows, cols);
	lcd->begin(LCD_COLS, LCD_ROWS);
	msg.enableLCDOutput(lcd);
#endif
	//lcd->clear();
	//lcd->print(F("PRB rev."));
	//lcd->println(_revision);
	//lcd->print(F("SW rev."));
	//lcd->print(BOARD_SW_REV);
	msg.clr();
	msg.out(F("PRB rev."));
	msg.outln(_revision);
	msg.out(F("SW rev."));
	msg.outln(BOARD_SW_REV);
	msg.disableLCDOutput();
	//delay(100);

	return true;
}

void Board_Sys80b::outputReset() {
	writeSolenoids(0);
	for (int i = 0; i < 12; i++) write4Lamps(i, 0);
	writeDisplayAndSoundReset(true);
	delay(50);
	writeDisplayAndSoundReset(false);
	writeSound(0xf); // reset sound output (active low)
}

uint16_t Board_Sys80b::incrementPowerOnCounter() {
	if (!NVData->ready()) {
		msg.warn(F("NVData not available!"), CLASS_NAME, "incrementPowerOnCounter");
		return 0;
	}
	uint16_t poc = NVData->readWord(0);
	if (poc++ == 0) restoreFactorySettings(false); // on first bootup only
	NVData->writeWord(0, poc);
	NVData->update();
	return poc;
}

void Board_Sys80b::lcdprn(byte line, const char* st, byte col) {
	if (_lcd == nullptr) return;
	_lcd->setCursor(col, line);
	_lcd->print(st);
}

void Board_Sys80b::lcdprn(byte line, const __FlashStringHelper* st, byte col) {
	if (_lcd == nullptr) return;
	_lcd->setCursor(col, line);
	_lcd->print(st);
}

void Board_Sys80b::lcdprn(byte line, String& st, byte col) {
	if (_lcd == nullptr) return;
	_lcd->setCursor(col, line);
	_lcd->print(st);
}

void Board_Sys80b::writeDisplayAndSoundReset(bool b) {
	if (_revision == 0) digitalWriteFast(D_RES_PIN, (b) ? LOW : HIGH);
	displayAndSound_IO->mcpWritePB(b ? 0 : 0x80); // 8th bit of sound output
}

void Board_Sys80b::writeDisplayLD(byte b) {
	digitalWriteFast(D_LD1_PIN, ((b & 1) ? HIGH : LOW));
	digitalWriteFast(D_LD2_PIN, ((b & 2) ? HIGH : LOW));
}

ButtonID Board_Sys80b::readButtons() {
	nextButton->update();
	enterButton->update();
	backButton->update();
	if (nextButton->fallingEdge()) return NEXT_BUTT;
	if (enterButton->fallingEdge()) return ENTER_BUTT;
	if (backButton->fallingEdge()) return BACK_BUTT;
	return NONE_BUTT;
}

bool Board_Sys80b::readButtonsChange() {
	//nextButton->update();
	//if (nextButton->risingEdge() || nextButton->fallingEdge()) return true;
	//enterButton->update();
	//if (enterButton->risingEdge() || enterButton->fallingEdge()) return true;
	//backButton->update();
	//if (backButton->risingEdge() || backButton->fallingEdge()) return true;
	if (nextButton->update()) return true;
	if (enterButton->update()) return true;
	if (backButton->update()) return true;
	return false;
}

byte Board_Sys80b::_readHwRev() {
	byte rev = 0;
	if (digitalRead(REV_D0_PIN) == LOW) rev += 1;
	if (digitalRead(REV_D1_PIN) == LOW) rev += 2;
	return rev;
}

//void Board_Sys80b::writeDisplayAddr(byte adr) {
//	digitalWriteFast(D_LD1_PIN, ((adr & 1) != 0) ? HIGH : LOW);
//	digitalWriteFast(D_LD2_PIN, ((adr & 2) != 0) ? HIGH : LOW);
//}

// adr = 0..11
void Board_Sys80b::write4Lamps(byte adr, byte states) {
	if (adr >= 12) return;
	lamps_IO->mcpWrite((mux16[adr] << 4) | ((states) & 0xf));
}

void Board_Sys80b::restoreFactorySettings(bool immediateUpdate) {
	msg.out(F("* Restoring factory settings... "));
	//NVData->clearAll();
	NVData->writeBytes(2u, factoryStatsBytes, 74);
	NVData->writeBytes(SETTINGS_ADR, factorySettingsOpSw, 4);
	msg.outln(F("ok."));
	if (immediateUpdate) NVData->update();
}

byte Board_Sys80b::getSettingByte(byte addr) {
	if (addr > 3) return 0;
	return NVData->readByte(SETTINGS_ADR + addr);
}

byte Board_Sys80b::getSettingBit(byte bitPos) {
	bitPos &= 0x1fu; // = 0..31
	byte _byte = bitPos >> 3;
	byte _bit = bitPos & 7u;
	return (NVData->readByte(_byte) & (1 << _bit)) ? 1 : 0;
}

bool Board_Sys80b::saveSettingByte(byte addr, byte byteMask, byte value) {
	if (addr > 3) return false;
	byte v = getSettingByte(addr);
	v = (v & (~byteMask)) | (value & byteMask);
	if (v == value) return false;
	NVData->writeByte(SETTINGS_ADR + addr, v);
	return true;
}

bool Board_Sys80b::saveSettingBit(byte bitPos, bool state) {
	byte v, _bit, addr;
	if (bitPos > 31) return false;
	addr = bitPos >> 3;
	_bit = bitPos % 8;
	v = getSettingByte(addr);
	if (state) v = v | (1 << _bit);
	else v = v & (~(1 << _bit));
	NVData->writeByte(SETTINGS_ADR + addr, v);
	return true;
}


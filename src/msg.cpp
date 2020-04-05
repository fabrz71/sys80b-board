#include "Msg.h"

#include <SD_t3.h>
#include <SD.h>
#include "MemoryFree.h"
#include "LiquidCrystalFast.h"

//namespace Msg {
//	const char* WARN_MSG = "Warning: ";
//	const char* ERR_MSG = "ERROR: ";
//	const int msgDelay = 500;
//	LiquidCrystalFast* lcdpt = nullptr;
//	bool lcdOutputEnabled = false;
//	bool serialOutputEnabled = false;
//}

Msg::Msg() {
	lcd = nullptr;
	lcdOutputEnabled = false;
	serialOutputEnabled = false;
	logFileEnabled = false;
	normalOutputToLog = false;
	logFileName = DEF_LOGFILE;
	//delay = 0; // delay disabled
	lcd = nullptr;
	mdelay = DEF_DELAY;
}

Msg::Msg(LiquidCrystalFast& lcdispl) {
	lcd = &lcdispl;
	lcdOutputEnabled = false;
}

void Msg::enableLCDOutput() { 
	if (lcd != nullptr) lcdOutputEnabled = true;
	else warn(F("Can't enable undefined LCD device!"), CLASS_NAME);
}

void Msg::clr() {
	if (lcdOutputEnabled) {
		lcd->clear();
		lcd->setCursor(0, 0);
	}
	if (serialOutputEnabled) Serial.println(F("---"));
	if (mdelay) delay(mdelay);
}

void Msg::out(const char *txt) {
	if (lcdOutputEnabled) lcd->print(txt);
	if (serialOutputEnabled) Serial.print(txt);
	if (normalOutputToLog) _writeLog(txt);
	if (mdelay) delay(mdelay);
}

void Msg::out(String& txt) {
	if (lcdOutputEnabled) lcd->print(txt);
	if (serialOutputEnabled) Serial.print(txt);
	if (normalOutputToLog) _writeLog(txt);
	if (mdelay) delay(mdelay);
}

void Msg::out(const __FlashStringHelper *txt) {
	if (lcdOutputEnabled) lcd->print(txt);
	if (serialOutputEnabled) Serial.print(txt);
	if (normalOutputToLog) _writeLog(txt);
	if (mdelay) delay(mdelay);
}

void Msg::out(uint32_t n, int base) {
	if (lcdOutputEnabled) lcd->print(n);
	if (serialOutputEnabled) Serial.print(n, base);
	if (normalOutputToLog) {
		String st = String(n, base);
		_writeLog(st);
	}
	if (mdelay) delay(mdelay);
}

void Msg::outln(const char *txt) {
	if (lcdOutputEnabled) lcd->println(txt);
	if (serialOutputEnabled) Serial.println(txt);
	if (normalOutputToLog) _writeLogLn(txt);
	if (mdelay) delay(mdelay);
}

void Msg::outln(String& txt) {
	if (lcdOutputEnabled) lcd->println(txt);
	if (serialOutputEnabled) Serial.println(txt);
	if (normalOutputToLog) _writeLogLn(txt);
	if (mdelay) delay(mdelay);
}

void Msg::outln(const __FlashStringHelper *txt) {
	if (lcdOutputEnabled) lcd->println(txt);
	if (serialOutputEnabled) Serial.println(txt);
	if (normalOutputToLog) _writeLogLn(txt);
	if (mdelay) delay(mdelay);
}

void Msg::outln(uint32_t n, int base) {
	if (lcdOutputEnabled) lcd->println(n);
	if (serialOutputEnabled) Serial.println(n, base);
	if (normalOutputToLog) {
		String st = String(n, base);
		_writeLogLn(st);
	}
	if (mdelay) delay(mdelay);
}

void Msg::log(const char* txt, const char* className, const char* callerName) {
	buff = txt;
	log(buff, className, callerName);
}
void Msg::log(const __FlashStringHelper* txt, const char* className, const char* callerName) {
	buff = txt;
	log(buff, className, callerName);
}

void Msg::log(String& txt, const char* className, const char* callerName) {
	if (!logFileEnabled) return;
	if (className != nullptr) {
		_buildFullMsg(buff, txt, className, callerName);
		_writeLogLn(buff);
	}
	else _writeLogLn(txt);
}

//void Msg::lcdprn(byte line, const char *st) {
//	lcd->setCursor(0, line);
//	lcd->print(st);
//	if (msgDelay) delay(msgDelay);
//}
//
//void Msg::lcdprn(byte line, String& st) {
//	lcd->setCursor(0, line);
//	lcd->print(st);
//	if (msgDelay) delay(msgDelay);
//}

void Msg::warn(const char* txt, const char* className, const char* callerName) {
	String buff = txt;
	warn(buff, className, callerName);
}

void Msg::warn(String& txt, const char* className, const char* callerName) {
	String str = WARN_MSG;
	str += txt;
	_buildFullMsg(buff, str, className, callerName);
	outln(buff);
	log(buff);
}

void Msg::warn(const __FlashStringHelper* txt, const char* className, const char* callerName) {
		buff = txt;
		warn(buff, className, callerName);
}

void Msg::err(const char* txt, const char* className, const char* callerName) {
	buff = txt;
	err(buff, className, callerName);
}

void Msg::err(String& txt, const char* className, const char* callerName) {
	String str = ERR_MSG;
	str += txt;
	_buildFullMsg(buff, str, className, callerName);
	outln(buff);
	log(buff);
}

void Msg::err(const __FlashStringHelper* txt, const char* className, const char* callerName) {
	String buff = txt;
	err(buff, className, callerName);
}

void Msg::prFreeMem() {
	buff = String(freeMemory()) + " bytes free.";
	outln(buff); 
	log(buff);
}

void Msg::_buildFullMsg(String& destStr, String& msg, const char* className, const char* funcName) {
	destStr = "";
	if (className != nullptr) {
		destStr = className;
		if (funcName != nullptr) {
			destStr += "::";
			destStr += funcName;
			destStr += "()";
		}
		destStr += separator;
	}
	destStr += msg;
}

// appends log text-line
bool Msg::_writeLog(const char* msg) {
	File f = SD.open(logFileName.c_str(), FILE_WRITE);
	if (!f) return false;
	f.print(msg);
	f.close();
	return true;
}

// appends log text-line
bool Msg::_writeLog(const __FlashStringHelper* msg) {
	File f = SD.open(logFileName.c_str(), FILE_WRITE);
	if (!f) return false;
	f.print(msg);
	f.close();
	return true;
}

// appends log text-line
bool Msg::_writeLogLn(const char* msg) {
	File f = SD.open(logFileName.c_str(), FILE_WRITE);
	if (!f) return false;
	f.println(msg);
	f.close();
	return true;
}

// appends log text-line
bool Msg::_writeLogLn(const __FlashStringHelper* msg) {
	File f = SD.open(logFileName.c_str(), FILE_WRITE);
	if (!f) return false;
	f.println(msg);
	f.close();
	return true;
}

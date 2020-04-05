#ifndef _msg_h_
#define _msg_h_

#include "Arduino.h"

class LiquidCrystalFast;

class Msg {
public:
	const char* CLASS_NAME = "Msg";
	const char* WARN_MSG = "> WARNING: ";
	const char* ERR_MSG = ">> ERROR: ";
	const char* DEF_LOGFILE = "LOG.TXT"; // log file created on SD card
	const int DEF_DELAY = 100;
	bool normalOutputToLog;

protected:
	const char* separator = ": ";
	uint16_t mdelay;
	bool lcdOutputEnabled;
	bool serialOutputEnabled;
	bool logFileEnabled;
	LiquidCrystalFast* lcd;
	String logFileName;
	String buff;

public:
	Msg();
	Msg(LiquidCrystalFast& lcdispl);
	inline void enableLCDOutput(LiquidCrystalFast& lcdispl) { 
		lcd = &lcdispl; lcdOutputEnabled = true; 
	}
	void enableLCDOutput(); // to call only after enableLCDOutput(LiquidCrystalFast*)
	inline void enableLCDOutput(LiquidCrystalFast* lcdispl) { 
		lcd = lcdispl; lcdOutputEnabled = true; }
	inline void disableLCDOutput() { lcdOutputEnabled = false; }
	inline void enableSerialOutput() { serialOutputEnabled = true; }
	inline void disableSerialOutput() { serialOutputEnabled = false; }
	inline void enableLogOutput(const char* name = nullptr) { 
		logFileName = (name == nullptr) ? CLASS_NAME : name; logFileEnabled = true; }
	inline void disableLogOutput() { logFileEnabled = false; }
	inline void setPostDelay(uint16_t delay) { mdelay = delay; }
	void clr();
	void out(const char* txt);
	void out(String& txt);
	void out(const __FlashStringHelper* txt);
	void out(uint32_t n, int base = DEC);
	void outln(const char* txt);
	void outln(String& txt);
	void outln(const __FlashStringHelper* txt);
	void outln(uint32_t n, int base = DEC);
	void log(const char* txt,
		const char* className = nullptr, const char* callerName = nullptr);
	void log(const __FlashStringHelper* txt,
		const char* className = nullptr, const char* callerName = nullptr);
	void log(String& txt, const char* className = nullptr, const char* callerName = nullptr);
	//void lcdprn(byte line, const char* st);
	//void lcdprn(byte line, String& st);
	void warn(const char* txt, const char* className = nullptr, const char* callerName = nullptr);
	void warn(String& txt, const char* className = nullptr, const char* callerName = nullptr);
	void warn(const __FlashStringHelper* txt, const char* className = nullptr, const char* callerName = nullptr);
	void err(const char* txt, const char* className = nullptr, const char* callerName = nullptr);
	void err(String& txt, const char* className = nullptr, const char* callerName = nullptr);
	void err(const __FlashStringHelper* txt, const char* className = nullptr, const char* callerName = nullptr);
	void prFreeMem();

protected:
	void _buildFullMsg(String& destStr, String& msg, const char* className, const char* funcName);
	bool _writeLog(const char* txt);
	bool _writeLog(const __FlashStringHelper* txt);
	inline bool _writeLog(String& txt) { return _writeLog(txt.c_str()); }
	bool _writeLogLn(const char* txt);
	bool _writeLogLn(const __FlashStringHelper* txt);
	inline bool _writeLogLn(String& txt) { return _writeLogLn(txt.c_str());	}
};

#endif

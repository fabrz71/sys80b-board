/* *** Gottileb System 80/B SIM PRB ***
* (SiMulation Pinball Replacement control Board)
* software for Teensy 3.x board developed with Arduino IDE
* under GNU General Public Livence v3
* ---
* FLASH MEMORY DATA I/O
* ---
* Functions for pinball non-volatile data retrieve/store.
*/

#include "NVData.h"
#include <EEPROM.h>

/** attenzione: MANCA supporto dispositivo FRAM **/

NVData_class::NVData_class() {
	immediateWriteMode = false;
	_ready = false;
	_fileName = (char*)DEFAULT_FILENAME;
	_fileBaseAdr = 0;
}

NVData_class::~NVData_class() {
	update();
	delete[] _data;
}

//bool NVData_class::begin(Device d, uint16_t size, uint8_t cs_pin) {
//	String st;
//	const char* hdr = "begin";
//
//	device = d;
//	if (size > MAX_SIZE) {
//		st = "NVData size ";
//		st += size;
//		st += " too big: trimmed to ";
//		st += MAX_SIZE;
//		_warnMsg(hdr, st);
//		size = MAX_SIZE;
//	}
//	_size = size;
//	_data = new byte[_size];
//	_ready = false;
//
//	clearAll();
//	switch (device) {
//	case EEPROM_NVD:
//		break;
//	case FRAM_NVD:
//		_warnMsg(hdr, notImplemStr);
//		break;
//	case SDFILE_NVD:
//		if (cs_pin == 0) {
//			_warnMsg(hdr, "undefined SPI DS CS pin!");
//			return false;
//		}
//		if (!SD.begin(cs_pin)) {
//			_warnMsg(hdr, "cannot init SD device");
//			return false;
//		}
//		//if (!SD.exists(fileName)) {
//		//	st = cantReadFileStr + String(".. creating new.. ");
//		//	_warnMsg(hdr, st);
//		//	if (!_createEmptyFile(size)) {
//		//		_warnMsg(hdr, cantWriteFileStr);
//		//		return false;
//		//	}
//		//	else Serial.println("ok.");
//		//}
//		break;
//	default:
//		_warnMsg(hdr, "Unknown device type!");
//		return false;
//	}
//
//	_pendingChanges = false;
//	_ready = _loadData();
//	if (!_ready) _warnMsg(hdr, "Can't load data!");
//	//else Serial.println("begin completed.");
//	return _ready;
//}

bool NVData_class::begin(Device d, uint16_t size, byte cs_pin) {
	String st;
	const char* hdr = "begin";

	device = d;
	if (size > MAX_SIZE) {
		st = "NVData size ";
		st += size;
		st += " too big: trimmed to ";
		st += MAX_SIZE;
		_warnMsg(hdr, st);
		size = MAX_SIZE;
	}
	_size = size;
	_data = new byte[_size];
	_ready = false;

	clearAll();
	if (device == FRAM_NVD) {
		_warnMsg(hdr, notImplemStr);
		return false;
	}
	if (device == SDFILE_NVD) {
		if (cs_pin == 0) {
			_warnMsg(hdr, "undefined SPI DS CS pin!");
			return false;
		}
		//if (!SD.begin(cs_pin)) {
		//	_warnMsg(hdr, "cannot init SD device!");
		//	return false;
		//}
	}
	_pendingChanges = false;
	_ready = _loadData();
	if (!_ready) _warnMsg(hdr, cantReadDataStr);
	//else Serial.println("begin completed.");
	return _ready;
}

byte NVData_class::readByte(uint16_t addr) {
	if (!_check("readByte", addr)) return 0;
	return _data[addr];
}

void NVData_class::writeByte(uint16_t addr, byte val) {
	const char* hdr = "writeByte";
	if (!_check(hdr, addr)) return;
	if (_data[addr] == val) {
		_warnMsg(hdr, updatedValsStr);
		return;
	}
	_data[addr] = val;
	if (immediateWriteMode) _saveData(); else _pendingChanges = true;
}

void NVData_class::writeBytes(uint16_t addr, const byte* values, uint16_t size) {
	uint16_t i;
	const char* hdr = "writeBytes";

	if (!_check(hdr, addr)) return;
	if (size == 0) {
		String st = wrongSizeStr + String(" 0");
		_warnMsg(hdr, st);
		return;
	}
	if (addr + size > _size) {
		String st = wrongSizeStr;
		st += size;
		st += ": trimming to ";
		size = _size - addr;
		st += size;
		_warnMsg(hdr, st);
	}

	for (i = addr; i < addr + size; i++) _data[i] = values[i-addr];
	if (immediateWriteMode) _saveData(); else _pendingChanges = true;
}

uint16_t NVData_class::readWord(uint16_t addr) {
	const char* hdr = "readWord";
	if (!_check(hdr, addr, _size-2)) return 0;
	uint16_t data = _data[addr++];
	data |= ((uint16_t)_data[addr]) << 8;
	return data;
}

void NVData_class::writeWord(uint16_t addr, uint16_t val) {
	const char* hdr = "writeWord";
	if (!_check(hdr, addr, _size-2)) return;
	if (readWord(addr) == val) {
		_warnMsg(hdr, updatedValsStr);
		return;
	}
	_data[addr++] = (byte)(val & 0x00ff);
	 _data[addr] = (byte)((val & 0xff00) >> 8);
	if (immediateWriteMode) _saveData(); else _pendingChanges = true;
}

uint32_t NVData_class::readDWord(uint16_t addr) {
	const char* hdr = "readDWord";
	if (!_check(hdr, addr, _size-4)) return 0;
	uint32_t data = 0;
	uint16_t endAddr = addr + 3;
	while (endAddr >= addr) {
		data <<= 8;
		data |= _data[endAddr--];
	}
	return data;
}

void NVData_class::writeDWord(uint16_t addr, uint32_t val) {
	const char* hdr = "writeDWord";
	if (!_check(hdr, addr, _size-4)) return;
	if (readDWord(addr) == val) {
		_warnMsg(hdr, updatedValsStr);
		return;
	}
	uint16_t eAddr = addr + 3;
	while (addr <= eAddr) {
		_data[addr++] = (byte)(val & 0xff);
		val >>= 8;
	}
	if (immediateWriteMode) _saveData(); else _pendingChanges = true;
}

void NVData_class::dump(uint16_t fromAdr, uint16_t toAdr) {
	byte n;
	const char* hdr = "dump";
	String st;

	if (!_check(hdr, fromAdr)) return;
	if (toAdr == 0) toAdr = _size - 1;
	if (toAdr <= fromAdr) {
		st = invalidAddrStr;
		st += ": from " + String(fromAdr, HEX) + ", to " + String(toAdr, HEX);
		_warnMsg(hdr, st);
		return;
	}

	Serial.printf("* NVData memory dump(%x, %x):\n", fromAdr, toAdr);
	uint16_t startIdx = fromAdr & 0xfff0;
	uint16_t stopIdx = toAdr | 0xf;
	for (int i = startIdx; i <= stopIdx; i++) {
		if ((i & 0x0F) == 0) {
			Serial.print(i, HEX);
			Serial.print(": ");
		}
		n = _data[i];
		if (i >= fromAdr && i <= toAdr) {
			st = String(n, HEX);
			if (st.length() == 1) st = "0" + st;
			Serial.print(st);
		}
		else Serial.print("--");
		if ((i & 0x0f) == 0x0f) Serial.println(); else Serial.print(" ");
	}
	Serial.println();
}

void NVData_class::update() {
	if (!immediateWriteMode && _pendingChanges) _saveData();
	else {
		if (!_pendingChanges) _warnMsg("update", updatedValsStr);
		//Serial.print("immediateWriteMode = ");
		//Serial.print(immediateWriteMode);
		//Serial.print("; _pendingChanges = ");
		//Serial.println(_pendingChanges);
	}
}

//bool NVData_class::ready() {
//	return _ready;
//}

bool NVData_class::_createEmptyFile(uint16_t size) {
	File f = SD.open(_fileName, FILE_WRITE);
	if (f) {
		for (uint16_t i = 0; i < size; i++) f.write((byte)0);
		f.close();
	}
	//else _warnMsg("_createEmptyFile", "Can't create settings file!");
	return (bool)f;
}

bool NVData_class::_loadData() {
	const char* hdr = "_loadData";
	uint32_t sz;

	switch (device) {
	case EEPROM_NVD:
		for (uint16_t i = 0; i < _size; i++) _data[i] = EEPROM.read(i);
		break;
	case SDFILE_NVD: // reads file tail
		_fileBaseAdr = 0;
		_f = SD.open(_fileName, FILE_READ);
		if (!_f) {
			_warnMsg(hdr, "can't open SD file");
			return false;
		}
		sz = _f.size();
		if (sz < _size) {
			_warnMsg(hdr, "target file too short: ignored");
			return false;
		}
		_fileBaseAdr = sz - _size;
		_f.seek(_fileBaseAdr);
		_f.read(_data, _size);
		_f.close();
		break;
	case FRAM_NVD:
		// TODO..
		_warnMsg(hdr, notImplemStr);
		break;
	}
	return true;
}

bool NVData_class::_saveData() {
	String hdr = "_saveData";
	Serial.println("- Saving data on NVData");
	if (!_ready) {
		_warnMsg(hdr, notReadyStr);
		return false;
	}

	switch (device) {
	case EEPROM_NVD:
		for (uint16_t i = 0; i < _size; i++) EEPROM.update(i, _data[i]);
		break;
	case SDFILE_NVD:
		//if (SD.exists(fileName)) SD.remove(fileName);
		_f = SD.open(_fileName, FILE_WRITE);
		if (!_f) {
			String msg = "Can't open file ";
			msg += _fileName;
			_warnMsg(hdr, msg);
			return false;
		}
		_f.write(_data, _size);
		_fileBaseAdr = _f.size() - _size;
		//Serial.print(hdr);
		//Serial.print(": _fileBaseAdr = ");
		//Serial.println(_fileBaseAdr);
		_f.close();
		break;
	case FRAM_NVD:
		_warnMsg(hdr, notImplemStr);
		//for (uint16_t i = 0; i < _size; i++) ; // TODO..
		break;
	}

	_pendingChanges = false;
	return true;
}

void NVData_class::_warnMsg(String header, String msg) {
	Serial.print("WARNING: NVData::");
	Serial.print(header);
	Serial.print("(): ");
	Serial.println(msg);
}

bool NVData_class::_check(String hdr, uint16_t addr, uint16_t addrLimit) {
	if (!_ready) {
		_warnMsg(hdr, notReadyStr);
		return false;
	}
	if (addrLimit == 0) addrLimit = _size - 1;
	if (addr > addrLimit) {
		_warnMsg(hdr, invalidAddrStr);
		String st = "(addr: " + addr;
		st += ")";
		_warnMsg(hdr, st);
		return false;
	}
	return true;
}

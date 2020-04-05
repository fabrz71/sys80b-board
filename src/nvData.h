/* *** Gottileb System 80/B SIM PRB ***
* (SIMulation Pinball Replacement control Board)
* software for Teensy 3.x board developed with Arduino IDE
* under GNU General Public Livence v3
* ---
* NON-VOLATILE DATA MEMORY I/O
* ---
* Functions for pinball non-volatile data retrieve/store.
*/

#ifndef _nvData_h
#define _nvData_h

#include "Arduino.h"
#include <SD_t3.h>
#include <SD.h>

// Presuppone che SD sia già inizializzata tramite chiamata SD.begin().
class NVData_class {
public:
    static const uint16_t MAX_SIZE = 1024;
    const char* DEFAULT_FILENAME = "NVRAM.BIN";
    const __FlashStringHelper* CLASS_NAME = F("NVData_class");

    const __FlashStringHelper* cantReadSDStr = F("Can't open/read SD file");
    const __FlashStringHelper* cantWriteSDStr = F("Can't write SD file");
    const __FlashStringHelper* invalidAddrStr = F("Invalid address");
    const __FlashStringHelper* wrongSizeStr = F("Wrong size");
    const __FlashStringHelper* notReadyStr = F("Not ready: missing begin() call?");
    const __FlashStringHelper* outBoundRWStr = 
        F("Out of bound multiple byte read/write: incomplete operation");
    const __FlashStringHelper* updatedValsStr = F("Values already up to date");
    const __FlashStringHelper* notImplemStr = F("FRAM not implemented yet");
    const __FlashStringHelper* cantReadDataStr = F("Can't load data!");

    enum Device { EEPROM_NVD, SDFILE_NVD, FRAM_NVD };

    Device device;
    bool immediateWriteMode;

protected:
    char* _fileName;
    
    File _f;
    bool _ready;
    byte* _data;
    uint16_t _size;
    bool _pendingChanges;
    uint32_t _fileBaseAdr;

public:
    NVData_class();
    ~NVData_class();
    bool begin(Device d, uint16_t size, byte cs_pin = 0);
    //bool begin(Device d, byte size);
    byte readByte(uint16_t addr);
    inline byte readByteFast(uint16_t addr) { return (addr < _size) ? _data[addr] : 0; }
    void writeByte(uint16_t addr, byte val);
    void writeBytes(uint16_t addr, const byte* values, uint16_t size);
    uint16_t readWord(uint16_t addr);
    void writeWord(uint16_t addr, uint16_t val);
    uint32_t readDWord(uint16_t addr);
    void writeDWord(uint16_t addr, uint32_t val);
    void dump(uint16_t from = 0, uint16_t to = 0);
    void update();
    inline void clearAll() { for (uint16_t i = 0; i < _size; i++) _data[i] = 0; }
    //bool ready();
    inline bool ready() { return _ready; }

protected:
    bool _createEmptyFile(uint16_t size);
    bool _loadData();
    bool _saveData();
    void _warnMsg(String from, String msg);
    bool _check(String hdr, uint16_t addr, uint16_t addrLimit = 0);
};

//static NVData_class NVData;

#endif
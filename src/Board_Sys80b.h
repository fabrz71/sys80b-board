#ifndef _Board_sys80b_h_
#define _Board_sys80b_h_

#include "Arduino.h"

#include <SD_t3.h>
#include <SD.h>
#include <LiquidCrystalFast.h>
#include "PIO.h"
#include "nvData.h"
#include "Led8x8.h"
#include "msg.h"

//// supports rev. 0, 1
//#define BOARD_REV 0
#define NO_LCD_MOUNTED

class SPISettings;
class Bounce;

enum ButtonID { NONE_BUTT, BACK_BUTT, ENTER_BUTT, NEXT_BUTT };
enum lg_mode { LG_OFF, LG_SWITCHES, LG_LAMPS };

//Msg msg;

class Board_Sys80b {
public:
    const char* CLASS_NAME = "Board_Sys80b";

    static const uint32_t SERIAL_BAUDRATE = 115200u; // serial communication through USB
    static const uint32_t SPI_CLK = 100000u; // SPI clock [Hz]
    static const uint16_t DEBOUNCE_T = 100u; // debounce check time
    static const uint16_t SETTINGS_ADR = 100u; // NV Data settings' memory address
    static const byte LCD_ROWS = 2; // LCD display rows
    static const byte LCD_COLS = 20; // LCD display cols

    const uint16_t mux16[16] = // 16 bit mux outputs
    { 0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
    0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000, 0x8000 };

    enum Pins { // Teensy I/O pins definition
        SLAM_PIN = 2, // INPUT, active low - slam switch
        NEXTB_PIN = 3, // INPUT PULLUP, active high - "Next" control button
        ENTRB_PIN = 4, // INPUT PULLUP, active high - "Enter" control button
        BACKB_PIN = 5, // INPUT PULLUP, active high - "Back" control button
        LCD_RS_PIN = 8, // OUTPUT - LCD display reset
        LCD_EN_PIN = 9, // OUTPUT - LCD display enable
        SD_SS_PIN = 10, // OUTPUT, active low - SPI SD select
        REV_D0_PIN = 11, // INPUT PULLUP, hardware revision number (bit 0)
        REV_D1_PIN = 12, // INPUT PULLUP, hardware revision number (bit 0)
        GPIO_SS_PIN = 14, // OUTPUT, active low - baseAPI.h - SPI GPIO select
        LG_SS_PIN = 15, // OUTPUT, active low - ledGrid.h - SPI Led Grid select
        D_LD1_PIN = 16, // OUTPUT, active high - pinball display 1 select
        D_LD2_PIN = 17, // OUTPUT, active high - pinball display 2 select
        D_RES_PIN = 18, // OUTPUT, active high - pinball displays reset
        LCD_D7_PIN = 20, // OUTPUT - LCD display data bits..
        LCD_D6_PIN = 21,
        LCD_D5_PIN = 22,
        LCD_D4_PIN = 23
    };

    //enum InputType { RETURNS, SLAM_SW };
    //enum OutputType { STROBES, SOLENOIDS, SOUND, LAMPS, DISPL };

    const byte factorySettingsOpSw[4] = { // Factory "Op-Adj" switch settings values
        0b00000011, 0b00000111, 0b00000001, 0b11000010 };

    const byte factoryStatsBytes[74] = { // Factory stats values (75 bytes)
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x4b, 0x4c, 0x00, 0x80, 0x96, 0x98, 0x00, 0x00, 0x2d,
        0x31, 0x01, 0x00, 0x00, 0x60, 0xe3, 0x16, 0x00, 0x52, 0x41, 0x4d, 0x00, 0x80, 0x4f, 0x12, 0x00,
        0x52, 0x41, 0x4d, 0x00, 0x40, 0x42, 0x0f, 0x00, 0x52, 0x41, 0x4d, 0x00, 0x00, 0x35, 0x0c, 0x00,
        0x52, 0x41, 0x4d, 0x00, 0x20, 0xa1, 0x07, 0x00, 0x52, 0x41, 0x4d, 0x00 };

    SDClass* sd;
    Bounce* nextButton, * enterButton, * backButton;
    Led8x8* ledGrid;
    PIO* solenoids_IO, * lamps_IO, * switchMatrix_IO, * displayAndSound_IO;
    NVData_class* NVData;
    lg_mode ledGridMode;
    Msg msg;
    //CmdExecutor* executor;

private:
    SPISettings* _spiSettings;
    byte _revision;
    uint16_t _poc; // power-on counter
    LiquidCrystalFast* _lcd;

public:
    Board_Sys80b();
    ~Board_Sys80b();
    bool begin();
    void outputReset();
    ButtonID readButtons();
    bool readButtonsChange();
    inline byte getRevision() { return _revision; }
    void write4Lamps(byte adr, byte states);
    //void writeDisplayAddr(byte adr);
    void restoreFactorySettings(bool immediateUpdate);
    byte getSettingByte(byte addr);
    byte getSettingBit(byte bitPos);
    bool saveSettingByte(byte addr, byte byteMask, byte value);
    bool saveSettingBit(byte bitPos, bool state);
    uint16_t incrementPowerOnCounter();
    //bool readSerialInput();
    inline void lcdclr() { if (_lcd != nullptr) _lcd->clear(); }
    inline void lcdprn(const char* st) { if (_lcd != nullptr) _lcd->print(st); }
    inline void lcdprn(const __FlashStringHelper* st) { if (_lcd != nullptr) _lcd->print(st); }
    inline void lcdprn(String& st) { if (_lcd != nullptr) _lcd->print(st); }
    void lcdprn(byte line, const char* st, byte col = 0);
    void lcdprn(byte line, const __FlashStringHelper* st, byte col = 0);
    void lcdprn(byte line, String& st, byte col = 0);

    inline byte getSettingSwitch(byte n) { return getSettingBit(n - 1); }
    inline void setSettingSwitch(byte n, bool s) { saveSettingBit(n - 1, s); }
    void writeDisplayAndSoundReset(bool b);
    inline void writeSound(byte b) { displayAndSound_IO->mcpWritePB(0x80 | b); }
    inline void clearLampsOutput() { lamps_IO->mcpWrite(0); }
    inline void writeDisplayData(byte b) { displayAndSound_IO->mcpWritePA(b); }
    void writeDisplayLD(byte b);
    inline void writeDisplayLD1(bool b) { digitalWriteFast(D_LD1_PIN, (b ? HIGH : LOW)); }
    inline void writeDisplayLD2(bool b) { digitalWriteFast(D_LD2_PIN, (b ? HIGH : LOW)); }
    inline void writeStrobes(byte b) { switchMatrix_IO->mcpWritePA(b); }
    inline void writeSolenoids(uint16_t states) { solenoids_IO->mcpWrite(states); }
    inline byte readReturns() { return switchMatrix_IO->mcpReadPB(); }
    inline bool readSlamSw() { return (digitalReadFast(SLAM_PIN) == HIGH); }
    inline uint16_t getPowerOnCounter() { return _poc; }
    inline void resetPowerOnCounter() { NVData->writeWord(0, 0); _poc = 0; }
    inline uint16_t rand(uint16_t top) { return (uint16_t)(micros() % top); }
private:
    byte _readHwRev();
};

//extern Msg msg;

#endif
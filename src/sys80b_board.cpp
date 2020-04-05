/*
 Name:		sys80b_board.cpp
 Created:	15/03/2020 14:46:57
 Author:	Fabrizio
 Editor:	http://www.visualmicro.com
*/

#include "sys80b_board.h"

void test(Board_Sys80b board) {
    int i;
    Serial.println("** Board tests **");

    Serial.println("- solenoids");
    for (i = 0; i < 9; i++) {
        board.writeSolenoids(1 << i);
        delay(100);
    }
    for (i = 8; i >= 0; i--) {
        board.writeSolenoids(1 << i);
        delay(100);
    }
    board.writeSolenoids(0);
    Serial.println("- lamps");
    for (i = 0; i < 48; i++) {
        board.write4Lamps(i >> 2, i & 0xf);
        delay(100);
    }
    board.clearLampsOutput();
    Serial.println("- sound");
    for (i = 0; i < 32; i++) {
        board.writeSound(i);
        delay(100);
    }
    board.writeSound(0);
    for (i = 0; i < 255; i++) {
        board.ledGrid->setRow(i & 7u, i);
        delay(20);
    }
    board.ledGrid->clear();
    if (board.getRevision() > 0) {
        board.lcdprn(0, "Hello world!");
        delay(100);
    }
    Serial.println("- non-volatile data");
    for (i = 0; i < 76; i++) {
        if ((i & 15) == 0) Serial.printf("%d:", i);
        Serial.printf(" %x", board.NVData->readByte(i));
        if ((i & 15) == 15) Serial.printf("\n");
        delay(20);
    }
    Serial.println();
    for (i = 0; i < 4; i++) {
        Serial.print(i);
        Serial.print(": ");
        Serial.println(board.getSettingByte(i), HEX);
        delay(20);
    }
}
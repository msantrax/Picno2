/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DS1307.h
 * Author: opus
 *
 * Created on 24 de Novembro de 2017, 22:33
 */

#ifndef DS1307_H
#define DS1307_H

#include "mbed.h"
#include "DateTime.h"


// RTC based on the DS1307 chip connected via I2C
class RtcDs1307
{
public:
    RtcDs1307(I2C &i2c);
    bool adjust(const DateTime& dt);
    bool isRunning();
    DateTime now();
    bool commit();
    uint8_t &operator[](uint8_t i) { return mRam[(i<sizeof(mRam)-1 ? i+1 : 0)]; };
protected:
    I2C &mI2c;
    uint8_t mRam[1+56]; // device register address + 56 bytes
};






#endif /* DS1307_H */


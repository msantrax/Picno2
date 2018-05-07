/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "mbed.h"
#include "DS1307.h"

#define DS1307_ADDRESS 0xD0

////////////////////////////////////////////////////////////////////////////////
// RtcDs1307 implementation

static uint8_t bcd2bin (uint8_t val) { return val - 6 * (val >> 4); }
static uint8_t bin2bcd (uint8_t val) { return val + 6 * (val / 10); }

RtcDs1307::RtcDs1307(I2C &i2c) : mI2c(i2c)
{
    // Read the RTC ram into the object RAM image
    mRam[0]=8;
    if(mI2c.write(DS1307_ADDRESS,(const char *)&mRam[0],1,true) == 0)
        mI2c.read(DS1307_ADDRESS,(char *)&mRam[1],sizeof(mRam)-1);
    else
        memset(mRam,0,sizeof(mRam));
}

bool RtcDs1307::commit()
{
    mRam[0] = 8; // device register address 
    return (mI2c.write(DS1307_ADDRESS,(const char *)mRam,sizeof(mRam)) == 0);
}

bool RtcDs1307::isRunning()
{   uint8_t i = 0;
    
    return (mI2c.write(DS1307_ADDRESS,(const char *)&i,sizeof(i),true) == 0
        && mI2c.read(DS1307_ADDRESS,(char *)&i,sizeof(i)) == 0
        && (i&0x80) == 0
        );
}

bool RtcDs1307::adjust(const DateTime& dt)
{   uint8_t buf[9] =
    {
        0 // device register address
        ,bin2bcd(dt.second()&0x7F) // make sure bit 7 (CH - Clock Halt) is off or the clock will be stopped
        ,bin2bcd(dt.minute())
        ,bin2bcd(dt.hour()&0x3F) // force 24h mode
        ,bin2bcd(0)
        ,bin2bcd(dt.day())
        ,bin2bcd(dt.month())
        ,bin2bcd(dt.year() - 2000)
        ,0 // turn off SQWO
    };
        
    return (mI2c.write(DS1307_ADDRESS,(const char *)buf,sizeof(buf)) == 0);
}

DateTime RtcDs1307::now()
{   uint8_t buf[7] = {0};
    
    if(mI2c.write(DS1307_ADDRESS,(const char *)&buf[0],1,true) == 0)
        mI2c.read(DS1307_ADDRESS,(char *)buf,sizeof(buf));

    return DateTime (
        bcd2bin(buf[6]) + 2000 // y
        ,bcd2bin(buf[5]) // m
        ,bcd2bin(buf[4]) // d
        ,bcd2bin(buf[2] & 0x3F) // hh - mask off 24h mode
        ,bcd2bin(buf[1]) // mm
        ,bcd2bin(buf[0] & 0x7F) // ss - mask off CH - Clock Halt
        );
}
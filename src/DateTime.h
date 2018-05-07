/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DateTime.h
 * Author: opus
 *
 * Created on 24 de Novembro de 2017, 22:34
 */

#ifndef DATETIME_H
#define DATETIME_H


#include "mbed.h"

// Simple general-purpose date/time class (no TZ / DST / leap second handling!)
class DateTime {
    
    public:
        DateTime (uint32_t t =0);
        DateTime (uint16_t year, uint8_t month, uint8_t day, uint8_t hour =0, uint8_t min =0, uint8_t sec =0);
        DateTime (const char* date, const char* time);
        uint16_t year() const       { return 2000 + yOff; }
        uint8_t month() const       { return m; }
        uint8_t day() const         { return d; }
        uint8_t hour() const        { return hh; }
        uint8_t minute() const      { return mm; }
        uint8_t second() const      { return ss; }
        uint8_t dayOfWeek() const;

        // 32-bit times as seconds since 1/1/1970
        uint32_t unixtime(void) const;

    protected:
        uint8_t yOff, m, d, hh, mm, ss;
    };


#ifdef WANT_RTC_MILLIS
// RTC using the internal millis() clock, has to be initialized before use
// NOTE: this clock won't be correct once the millis() timer rolls over (>49d?)
class RTC_Millis
{
public:
    static void begin(const DateTime& dt) { adjust(dt); }
    static void adjust(const DateTime& dt);
    static DateTime now();

protected:
    static long offset;
};
#endif

#endif /* DATETIME_H */


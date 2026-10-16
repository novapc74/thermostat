#include <Arduino.h>
#ifndef DISPLAY_ICON_H
#define DISPLAY_ICON_H
#include "Config.h"

// Маски сегментов для индикации режимов на 4-м разряде
const uint8_t SEG_ICON_TOP    = 0b00000001; // Верхняя черточка (¯) -> Выбор ТЕМП
const uint8_t SEG_ICON_MIDDLE = 0b01000000; // Средняя черточка (-) -> Выбор ВРЕМЕНИ
const uint8_t SEG_ICON_BOTTOM = 0b00001000; // Нижняя черточка (_)  -> Рабочий режим

class DisplayIcon {
public:
    void init() {
        pinMode(CLK, INPUT); 
        pinMode(DIO, INPUT);
        digitalWrite(CLK, LOW); 
        digitalWrite(DIO, LOW);
    }

    void clear() {
        start(); writeByte(0x40); stop();
        start(); writeByte(0xC0);
        for(int i=0; i<4; i++) writeByte(0x00);
        stop();
    }

    void update(int value, uint8_t iconMask, bool blinkState, bool forceBlink = false, bool dotState = false) {
        if (forceBlink && !blinkState) {
            clear();
            return;
        }

        const uint8_t digits[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f };
        
        start(); writeByte(0x40); stop(); 
        start(); writeByte(0xC0);          
        
        // Выводим 1-ю и 2-ю цифры числа
        writeByte(value >= 100 ? digits[(value / 100) % 10] : 0x00);
        writeByte(value >= 10  ? digits[(value / 10) % 10]  : 0x00);
        
        // 3-я цифра (единицы). ИСПРАВЛЕНО: Теперь точка подмешивается СЮДА (0x80 — маска точки DP)
        uint8_t unitDigitMask = digits[value % 10];
        if (dotState) unitDigitMask |= 0x80; 
        writeByte(unitDigitMask);
        
        // 4-й разряд — чистая иконка режима без паразитной засветки двоеточия
        writeByte(iconMask);
        stop();

        start(); writeByte(0x8f); stop(); 
    }

private:
    void bitDelay() { delayMicroseconds(50); }
    void start()    { pinMode(DIO, OUTPUT); bitDelay(); }
    void stop()     { pinMode(DIO, OUTPUT); bitDelay(); pinMode(CLK, INPUT); bitDelay(); pinMode(DIO, INPUT); bitDelay(); }
    
    bool writeByte(uint8_t b) {
        for(uint8_t i = 0; i < 8; i++) {
            pinMode(CLK, OUTPUT);
            bitDelay();
            if (b & 0x01) pinMode(DIO, INPUT); else pinMode(DIO, OUTPUT);
            bitDelay();
            pinMode(CLK, INPUT);
            bitDelay();
            b >>= 1;
        }
        pinMode(CLK, OUTPUT);
        pinMode(DIO, INPUT);
        bitDelay();
        pinMode(CLK, INPUT);
        bitDelay();
        (void)digitalRead(PIN_BTN_MINUS); 
        pinMode(CLK, OUTPUT);
        bitDelay();
        return true;
    }
};

static DisplayIcon myDisplay; 
#endif

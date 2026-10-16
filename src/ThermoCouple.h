#ifndef THERMO_COUPLE_H
#define THERMO_COUPLE_H
#include <Arduino.h>
#include "Config.h"

class ThermoCouple {
public:
    void init() {
#ifdef SIMULATOR_MODE
        // В режиме симулятора настраиваем пин SO (A0) как аналоговый вход
        pinMode(PIN_THERMO_SO, INPUT);
#else
        // В боевом режиме конфигурируем полноценную шину SPI под MAX6675
        pinMode(PIN_THERMO_CS, OUTPUT);
        pinMode(PIN_THERMO_SCK, OUTPUT);
        pinMode(PIN_THERMO_SO, INPUT);
        digitalWrite(PIN_THERMO_CS, HIGH); // Уводим чип в ожидание
#endif
        _lastReadTime = 0;
        _lastValidTemp = 0;
    }

    int readTemperature() {
        unsigned long currentMillis = millis();
        
        // Опрашиваем датчик согласно его внутренним таймингам
        if (currentMillis - _lastReadTime < 300) {
            return _lastValidTemp; 
        }
        _lastReadTime = currentMillis;

#ifdef SIMULATOR_MODE
        // --- ЛОГИКА СИМУЛЯТОРА: Чтение аналогового ползунка ---
        int rawAnalog = analogRead(PIN_THERMO_SO);
        _lastValidTemp = map(rawAnalog, 0, 1023, 0, 250);
#else
        // --- БОЕВАЯ ЛОГИКА ЖЕЛЕЗА: Чтение цифрового чипа MAX6675 по SPI ---
        digitalWrite(PIN_THERMO_CS, LOW); // Активируем чип
        delayMicroseconds(1);
        
        uint16_t spiData = 0;
        for (int i = 15; i >= 0; i--) {
            digitalWrite(PIN_THERMO_SCK, HIGH);
            delayMicroseconds(1);
            if (digitalRead(PIN_THERMO_SO)) {
                spiData |= (1 << i);
            }
            digitalWrite(PIN_THERMO_SCK, LOW);
            delayMicroseconds(1);
        }
        digitalWrite(PIN_THERMO_CS, HIGH); // Деактивируем чип
        
        // Проверка бита ошибки D2 (обрыв или отсутствие термопары)
        if (spiData & 0x04) {
            _lastValidTemp = 0; // Аварийная защита: сброс в ноль для отключения ТЭНа
        } else {
            // Сдвигаем биты D14..D3 и переводим в градусы Цельсия
            _lastValidTemp = (int)((spiData >> 3) * 0.25);
        }
#endif

        return _lastValidTemp;
    }

private:
    unsigned long _lastReadTime;
    int _lastValidTemp;
};

static ThermoCouple thermo; 
#endif

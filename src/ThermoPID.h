#ifndef THERMO_PID_H
#define THERMO_PID_H
#include <Arduino.h>
#include "Config.h"

class ThermoPID {
public:
    int pidOutput; // Переменная теперь принадлежит классу и доступна везде

    void init() {
        pinMode(PIN_LED_TRIAC, OUTPUT);
        _integral = 0; _prevInput = 0; _lastPIDTime = 0;
        pidOutput = 0;
    }

    void resetIntegral() { _integral = 0; }

    void compute(float currentT, float setT, bool powerAllowed) {
        unsigned long currentMillis = millis();
        if (currentMillis - _lastPIDTime < PID_DT) return; 
        _lastPIDTime = currentMillis;

        if (!powerAllowed) {
            analogWrite(PIN_LED_TRIAC, 0); 
            _integral = 0;
            pidOutput = 0; // Сбрасываем мощность
            return;
        }

        float error = setT - currentT;
        float pTerm = PID_KP * error;
        
        _integral += error * (PID_DT / 1000.0);
        float iTerm = PID_KI * _integral;
        if (iTerm < 0) { iTerm = 0; }
        if (iTerm > 255) { iTerm = 255; }

        float dTerm = -PID_KD * (currentT - _prevInput) / (PID_DT / 1000.0);
        _prevInput = currentT;

        float output = pTerm + iTerm + dTerm;
        if (output < 0) { output = 0; }
        if (output > 255) { output = 255; }

        pidOutput = (int)output; // Запоминаем текущую вычисленную мощность
        analogWrite(PIN_LED_TRIAC, pidOutput);
    }

private:
    float _integral;
    float _prevInput;
    unsigned long _lastPIDTime;
};

static ThermoPID heaterPID; // Переменная модуля
#endif

#include <Arduino.h>
#ifndef BUTTON_HOLD_H
#define BUTTON_HOLD_H

enum ButtonResult { BTN_NONE, BTN_CLICK, BTN_HOLD_TICK };

class ButtonHold {
public:
    ButtonHold(uint8_t pin) {
        _pin = pin; _isHeld = false; _wasPressed = false;
    }

    void init() {
        pinMode(_pin, INPUT_PULLUP);
    }

    ButtonResult checkState() {
        bool isPressedNow = (digitalRead(_pin) == LOW);
        unsigned long currentMillis = millis();

        if (isPressedNow && !_wasPressed) {
            _wasPressed = true;
            _pressTime = currentMillis;
            _isHeld = false;
            return BTN_CLICK; 
        }

        if (_wasPressed) {
            if (isPressedNow) {
                if (currentMillis - _pressTime >= 500) {
                    _isHeld = true;
                    if (currentMillis - _lastTickTime >= 100) { 
                        _lastTickTime = currentMillis;
                        return BTN_HOLD_TICK;
                    }
                }
            } else {
                _wasPressed = false;
                _isHeld = false;
            }
        }
        return BTN_NONE;
    }

private:
    uint8_t _pin;
    unsigned long _pressTime;
    unsigned long _lastTickTime;
    bool _isHeld;
    bool _wasPressed;
};
#endif

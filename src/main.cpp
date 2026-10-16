#include <Arduino.h>
#include <EEPROM.h>        
#include "Config.h"
#include "DisplayIcon.h"
#include "ButtonHold.h"
#include "ThermoPID.h"
#include "ThermoCouple.h"  

ButtonHold btnPlus(PIN_BTN_PLUS);
ButtonHold btnMinus(PIN_BTN_MINUS);
ButtonHold btnSet(PIN_BTN_SEL);

enum DeviceMode { MODE_TEMP, MODE_TIME, MODE_SET_TEMP, MODE_SET_TIME };
DeviceMode currentMode = MODE_TEMP;

int currentTemp = 0;
int setTemp = DEFAULT_TEMP;            
long defaultTimerSeconds = DEFAULT_TIME; 
long timerSeconds = DEFAULT_TIME;       

bool timerRunning = false;     
unsigned long timerCountdown = 0; 
unsigned long timerButtonsGlobal = 0;
unsigned long timerBlink = 0;
unsigned long lastInactivityTime = 0; 
unsigned long lastStartActionTime = 0; 
bool blinkState = false;
bool isSystemShutDown = false;        

unsigned long btnStartPressTime = 0;
bool btnStartWasPressed = false;
bool isLongPressExecuted = false;

// Переменные для серийного будильника
bool isAlarmActive = false;         
unsigned long alarmTimer = 0;       
int alarmPulseCount = 0;            
int alarmSeriesCount = 0;           
bool alarmStateState = false;       

// Переменные для логики мигания точки ПИД
unsigned long timerPidDot = 0;      
bool pidDotState = false;           

void setup() {
  myDisplay.init();
  heaterPID.init();
  thermo.init();       
  btnPlus.init();
  btnMinus.init();
  btnSet.init();
  
  pinMode(PIN_BTN_START, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT); 
  pinMode(PIN_LED_BLINK, OUTPUT); 

  int storedTemp = EEPROM.read(0);
  if (storedTemp >= LIMIT_MIN_TEMP && storedTemp <= LIMIT_MAX_TEMP) setTemp = storedTemp;
  
  long storedTime = 0; 
  EEPROM.get(2, storedTime); 
  if (storedTime >= LIMIT_MIN_TIME && storedTime <= LIMIT_MAX_TIME) {
    defaultTimerSeconds = storedTime; 
    timerSeconds = storedTime;
  }
  lastStartActionTime = millis(); 
}

void saveSettings() {
  EEPROM.update(0, setTemp);
  EEPROM.put(2, defaultTimerSeconds);
}

void loop() {
  unsigned long currentMillis = millis();
  
  currentTemp = thermo.readTemperature(); 

  // --- АВТООТКЛЮЧЕНИЕ ПРИ ПРОСТОЕ ---
  if (!timerRunning && (currentMillis - lastStartActionTime >= TIMEOUT_AUTO_OFF)) isSystemShutDown = true;

  // --- ЛОГИКА ТАЙМЕРА ОБРАТНОГО ОТСЧЕТА ---
  if (timerRunning && (currentMillis - timerCountdown >= 1000)) {
    timerCountdown = currentMillis;
    if (timerSeconds > 0) {
      timerSeconds--;
    } else {
      timerRunning = false; 
      currentMode = MODE_TEMP;
      isAlarmActive = true;
      alarmTimer = currentMillis;
      alarmPulseCount = 0;
      alarmSeriesCount = 0;
      alarmStateState = true;
      tone(PIN_BUZZER, BUZZER_FREQ); 
    }
  }

  // --- РАБОТА ПИД НАГРЕВАТЕЛЯ ---
  bool heatingAllowed = (currentMode != MODE_SET_TEMP && currentMode != MODE_SET_TIME && !isSystemShutDown);
  heaterPID.compute(currentTemp, setTemp, heatingAllowed);

// --- УМНАЯ АДАПТИВНАЯ ИНДИКАЦИЯ ТОЧКИ ПИД-РЕГУЛЯТОРА ---
if (heaterPID.pidOutput == 0) {
  pidDotState = false; 
  digitalWrite(PIN_LED_BLINK, LOW); // Выключаем светодиод на D2
} else {
  long halfPeriod = map(heaterPID.pidOutput, 1, 255, 250, 40);
  
  if (currentMillis - timerPidDot >= (unsigned long)halfPeriod) {
    timerPidDot = currentMillis;
    pidDotState = !pidDotState; 
    
    // Зеленый светодиод на пине D2 теперь будет четко моргать в такт ПИД ...
    digitalWrite(PIN_LED_BLINK, pidDotState ? HIGH : LOW); 
  }
}


  // Логику точки дисплея оставляем для фонового расчета
  if (heaterPID.pidOutput == 0) {
    pidDotState = false; 
  } else {
    long halfPeriod = map(heaterPID.pidOutput, 1, 255, 250, 40);
    if (currentMillis - timerPidDot >= (unsigned long)halfPeriod) {
      timerPidDot = currentMillis;
      pidDotState = !pidDotState; 
    }
  }


  // --- АВТОМАТИЧЕСКИЙ ВЫХОД ИЗ МЕНЮ НАСТРОЕК ---
  if ((currentMode == MODE_SET_TEMP || currentMode == MODE_SET_TIME) && (currentMillis - lastInactivityTime >= 2000)) {
    saveSettings();
    currentMode = MODE_TEMP;   
  }

  // --- ЛОГИКА КНОПКИ START/STOP ---
  bool btnStartPressed = (digitalRead(PIN_BTN_START) == LOW);
  if (btnStartPressed && !btnStartWasPressed) { btnStartWasPressed = true; btnStartPressTime = currentMillis; isLongPressExecuted = false; }
  if (btnStartWasPressed) {
    if (btnStartPressed && !isLongPressExecuted && (currentMillis - btnStartPressTime >= 1500)) { 
      timerRunning = false; timerSeconds = defaultTimerSeconds; currentMode = MODE_TEMP; lastStartActionTime = currentMillis; isLongPressExecuted = true;         
      isAlarmActive = false; noTone(PIN_BUZZER); 
    }
    if (!btnStartPressed) { 
      btnStartWasPressed = false;
      if (!isLongPressExecuted && (currentMillis - timerButtonsGlobal >= 200)) {
        timerButtonsGlobal = currentMillis;
        lastStartActionTime = currentMillis;
        
        if (timerSeconds == 0 || isAlarmActive) {
          isAlarmActive = false;
          noTone(PIN_BUZZER);
          timerRunning = false;
          timerSeconds = defaultTimerSeconds; 
          currentMode = MODE_TEMP;            
          isSystemShutDown = false;
        } 
        else if (timerSeconds > 0 && currentMode != MODE_SET_TEMP && currentMode != MODE_SET_TIME) {
          timerRunning = !timerRunning; 
          timerCountdown = currentMillis; 
          currentMode = MODE_TIME; 
          isSystemShutDown = false;            
        }
      }
    }
  }

  // --- ЛОГИКА КНОПКИ SET ---
  if (currentMillis - timerButtonsGlobal >= 200 && digitalRead(PIN_BTN_SEL) == LOW) {
    timerButtonsGlobal = currentMillis; timerRunning = false; heaterPID.resetIntegral();
    isAlarmActive = false; noTone(PIN_BUZZER); 
    
    if (currentMode == MODE_TEMP || currentMode == MODE_TIME) { currentMode = MODE_SET_TEMP; lastInactivityTime = currentMillis; } 
    else if (currentMode == MODE_SET_TEMP) { currentMode = MODE_SET_TIME; lastInactivityTime = currentMillis; } 
    else if (currentMode == MODE_SET_TIME) { saveSettings(); currentMode = MODE_TEMP; }
  }

  // --- ЛОГИКА КНОПОК ПЛЮС И МИНУС ---
  if (currentMode == MODE_SET_TEMP || currentMode == MODE_SET_TIME) {
    ButtonResult pRes = btnPlus.checkState();
    ButtonResult mRes = btnMinus.checkState();

    if (pRes != BTN_NONE) {
      lastInactivityTime = currentMillis;
      if (currentMode == MODE_SET_TEMP && setTemp < LIMIT_MAX_TEMP) setTemp++;
      if (currentMode == MODE_SET_TIME && defaultTimerSeconds < LIMIT_MAX_TIME) { defaultTimerSeconds++; timerSeconds = defaultTimerSeconds; }
    }
    if (mRes != BTN_NONE) {
      lastInactivityTime = currentMillis;
      if (currentMode == MODE_SET_TEMP && setTemp > LIMIT_MIN_TEMP) setTemp--;
      if (currentMode == MODE_SET_TIME && defaultTimerSeconds > LIMIT_MIN_TIME) { defaultTimerSeconds--; timerSeconds = defaultTimerSeconds; }
    }
  }

  // ИСПРАВЛЕНО: Теперь полупериод рассчитывается динамически на основе константы BLINK_PERIOD_SET из файла Config.h
  if (currentMillis - timerBlink >= (unsigned long)(BLINK_PERIOD_SET / 2)) { 
    timerBlink = currentMillis; 
    blinkState = !blinkState; 
  }

  // --- СЕРИЙНАЯ ЛОГИКА БУДИЛЬНИКА ----
  if (isAlarmActive) {
    if (alarmStateState) {
      if (currentMillis - alarmTimer >= ALARM_PULSE_ON) {
        noTone(PIN_BUZZER);
        alarmStateState = false;
        alarmTimer = currentMillis;
        alarmPulseCount++; 
      }
    } else {
      if (alarmPulseCount >= ALARM_PULSES_IN_SERIES) {
        if (currentMillis - alarmTimer >= ALARM_SERIES_GAP) {
          alarmPulseCount = 0; 
          alarmSeriesCount++;  
          
          if (alarmSeriesCount >= ALARM_MAX_SERIES) {
            isAlarmActive = false;
          } else {
            alarmStateState = true;
            alarmTimer = currentMillis;
            tone(PIN_BUZZER, BUZZER_FREQ);
          }
        }
      } else {
        if (currentMillis - alarmTimer >= ALARM_PULSE_OFF) {
          alarmStateState = true;
          alarmTimer = currentMillis;
          tone(PIN_BUZZER, BUZZER_FREQ);
        }
      }
    }
  }

  // --- ОТРИСОВКА ИНТЕРФЕЙСА НА ЭКРАНЕ ---
  if (isSystemShutDown) { myDisplay.update(0, 0x00, false); return; }

  switch (currentMode) {
    case MODE_TEMP:     myDisplay.update(currentTemp, SEG_ICON_BOTTOM, blinkState, false, pidDotState); break;
    case MODE_TIME:     myDisplay.update(timerSeconds, SEG_ICON_BOTTOM, blinkState, false, pidDotState); break;
    case MODE_SET_TEMP: myDisplay.update(setTemp, SEG_ICON_TOP, blinkState, true, false); break; 
    case MODE_SET_TIME: myDisplay.update(defaultTimerSeconds, SEG_ICON_MIDDLE, blinkState, true, false); break;
  }
}

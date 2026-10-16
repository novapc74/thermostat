#ifndef CONFIG_H
#define CONFIG_H

// =========================================================================
// ГЛАВНЫЙ ПЕРЕКЛЮЧАТЕЛЬ: РЕЖИМ СИМУЛЯТОРА / РЕАЛЬНОЕ ЖЕЛЕЗО
// =========================================================================
// Закомментируйте строчку ниже (поставьте // в начале), чтобы скомпилировать 
// прошивку под реальный термопресс с цифровой термопарой MAX6675.
#define SIMULATOR_MODE  

// =========================================================================
// НАСТРОЙКИ, ОГРАНИЧЕНИЯ И КОНСТАНТЫ
// =========================================================================
const int   LIMIT_MIN_TEMP = 70;      // Минимальный порог температуры (градусы)
const int   LIMIT_MAX_TEMP = 200;     // Максимальный порог температуры (градусы)
const int   DEFAULT_TEMP   = 155;     // Целевая температура по умолчанию

const long  LIMIT_MIN_TIME = 5;       // Минимальное время таймера (секунды)
const long  LIMIT_MAX_TIME = 180;     // Максимальное время таймера (3 минуты)
const long  DEFAULT_TIME   = 30;      // Время таймера по умолчанию

// Защита и звуковые настройки будильника термопресса
const unsigned long TIMEOUT_AUTO_OFF = 900000;  // 15 минут простоя (в мс)
const int   BUZZER_FREQ       = 2700;    // Высокая частота звука (3 кГц)
const int   ALARM_PULSE_ON    = 35;      // Длительность одного пика (35 мс)
const int   ALARM_PULSE_OFF   = 70;      // Пауза между пиками внутри серии (70 мс)
const int   ALARM_SERIES_GAP  = 400;     // Пауза между сериями (400 мс тишины)
const int   ALARM_PULSES_IN_SERIES = 4;  // Количество пиков в одной серии
const int   ALARM_MAX_SERIES  = 5;      // Всего 5 серий звука

// --- НАСТРОЙКА МИГАНИЯ ДИСПЛЕЯ ---
const int   BLINK_PERIOD_SET  = 600;     // ИСПРАВЛЕНО: Полный период мигания цифр в меню SET (мс)
const int   TIMEOUT_MENU_EXIT = 4000;

// Коэффициенты ПИД-регулятора мощности ТЭНа 2кВт
const float PID_KP = 4.5;    
const float PID_KI = 0.05;   
const float PID_KD = 1.5;    
const int   PID_DT = 100;    // Шаг расчета ПИД (мс)

// Назначение аппаратных пинов Arduino Nano
#define PIN_LED_TRIAC  3 
#define PIN_BTN_PLUS   4 
#define PIN_BTN_MINUS  5 
#define PIN_BTN_SEL    6 
#define CLK            7           
#define DIO            8
#define PIN_BUZZER     9    
#define PIN_BTN_START  A1  // Кнопка СТАРТ перенесена на пин A1
#define PIN_LED_BLINK  2   //  Дублирующий светодиод для наглядности

// --- АВТОМАТИЧЕСКОЕ РАСПРЕДЕЛЕНИЕ ПИНОВ ДАТЧИКА ---
#ifdef SIMULATOR_MODE
  // Для симулятора Wokwi используем аналоговый пин A0 под ползунок
  #define PIN_THERMO_SO   A0  
  #define PIN_THERMO_CS   -1  // Не используются в симуляторе
  #define PIN_THERMO_SCK  -1
#else
  // Для реального железа используем цифровые пины под шину SPI MAX6675
  #define PIN_THERMO_CS   10  
  #define PIN_THERMO_SO   12  
  #define PIN_THERMO_SCK  13  
#endif

#endif

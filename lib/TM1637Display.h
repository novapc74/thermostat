#ifndef __TM1637DISPLAY__
#define __TM1637DISPLAY__
#include <inttypes.h>

class TM1637Display {
public:
  TM1637Display(uint8_t pinCLK, uint8_t pinDIO);
  void setBrightness(uint8_t brightness, bool on = true);
  void setSegments(const uint8_t segments[], uint8_t length = 4, uint8_t pos = 0);
  void showNumberDec(int num, bool leading_zero = false, uint8_t length = 4, uint8_t pos = 0);
  void clear();
private:
  void bitDelay(); void start(); void stop(); bool writeByte(uint8_t b);
  uint8_t m_pinCLK; uint8_t m_pinDIO; uint8_t m_brightness;
};
#endif

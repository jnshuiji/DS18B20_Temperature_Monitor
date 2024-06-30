#ifndef _TM1637_H_
#define _TM1637_H_

#include "DS18B20.H"

sbit SDA = P2 ^ 0;
sbit SCL = P2 ^ 1;

void startTM1637();
void writeTM1637(uchar dat);
void ackTM1637();
void stopTM1637();
uchar keyTM1637();
void setluminanceTM1637(uchar luminance);
void displayTM1637(uchar grid, uchar sg);

#endif // !_TM1637_H_

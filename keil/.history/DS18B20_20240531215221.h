/***
 * @Author       : Quan Si
 * @Date         : 2024-05-23 14:18:30
 * @LastEditTime : 2024-05-31 13:25:49
 * @LastEditors  : Quan Si
 * @Description  :
 * @FilePath     : \keil\DS18B20.h
 */
#ifndef __DS18B20_H__
#define __DS18B20_H__

#include "INTRINS.H"
#include "REGX51.H"
#include "delay1ms.h"

sbit DQ = P3 ^ 7;
sbit buzzer = P2 ^ 2;

/*-----------------------------------------自定义数据类型-----------------------------------------*/
#ifndef _uchar_
#define _uchar_
typedef unsigned char uchar;
#endif

#ifndef _uint_
#define _uint_
typedef unsigned int uint;
#endif

/*-------------------------------------------函数声明-------------------------------------------*/
void delay5us(uchar n);
bit initDS18B20();
uchar readbyteDS18B20();
void writebyteDS18B20(uchar dat);
uchar calcrc_1byte(uchar abyte);
uchar calcrc_bytes(uchar *p, uchar len);
uint Read_temp_DS18B20();
uint Read_EEPROM_DS18B20();
void Write_Scratchpad_EEPROM_DS18B20(uchar TH, uchar TL, uchar precision);

/*-------------------------------------------ROM命令-------------------------------------------*/
#define Search_Rom 0xF0   // 搜索ROM
#define Read_Rom 0x33     // 读取ROM
#define Match_Rom 0x55    // 匹配ROM
#define Skip_Rom 0xCC     // 跳过ROM
#define Alarm_Search 0xEC // 警报搜索

/*-------------------------------------------功能命令-------------------------------------------*/
#define Convert_T 0x44         // 转换温度
#define Read_Scratchpad 0xBE   // 读取暂存寄存器:读取暂存寄存器中全部的数据，包括 CRC 字节。
#define Write_Scratchpad 0x4E  // 写入暂存寄存器:向暂存寄存器中的TH，TL 及配置寄存器（精度）写入数据
#define Copy_Scratchpad 0x48   // 拷贝暂存寄存器:将暂存寄存器中的TH，TL及配置寄存器中的数据拷贝到EEPROM中
#define Recall_EEPROM 0xB8     // 召回EEPROM:将EEPROM中的TH，TL及配置寄存器数据召回到暂存寄存器中
#define Read_Power_Supply 0xB4 // 读取供电模式:读取 DS18B20 的供电模式到控制器中

/*-------------------------------------------转换精度-------------------------------------------*/
#define precision_12 0x7F // 转换位数12，最大转换时间t=750ms
#define precision_11 0x5F // 转换位数11，最大转换时间t/2=375ms
#define precision_10 0x3F // 转换位数10，最大转换时间t/4=187.5ms
#define precision_9 0x1F  // 转换位数9，最大转换时间t/8=93.75ms

#endif
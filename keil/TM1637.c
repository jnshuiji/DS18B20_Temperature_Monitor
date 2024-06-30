/*
 * @Author: Quan Si
 * @Date: 2024-05-23 16:49:56
 * @LastEditTime: 2024-06-30 16:17:00
 * @LastEditors: Quan Si
 * @Description: TM1637驱动程序
 * @FilePath: \keil\TM1637.c
 */
#include "TM1637.h"

/**
 * @description: 开始
 * @return {*}
 */
void startTM1637()
{
	SCL = 1;
	_nop_();
	SDA = 1;
	_nop_();
	SDA = 0;
	_nop_();
	SCL = 0;
}

/**
 * @description: 向TM1637传输一字节数据
 * @param {uchar} dat
 * @return {*}
 */
void writeTM1637(uchar dat)
{
	uchar i = 0;
	for (; i < 8; i++)
	{
		_nop_();
		SDA = dat & 1; // 低在前
		SCL = 1;
		_nop_();
		SCL = 0;
		dat >>= 1;
	}
}

/**
 * @description: 等待TM1637应答
 * @return {*}
 */
void ackTM1637()
{
	_nop_();
	// while (SDA)
	// {
	// 	buzzer = 0;
	// 	delay1ms(200);
	// 	buzzer = 1;
	// 	delay1ms(200);
	// }
	buzzer = 1;
	SCL = 1;
	_nop_();
	SCL = 0;
}

/**
 * @description: 停止
 * @return {*}
 */
void stopTM1637()
{
	SCL = 0;
	_nop_();
	SDA = 0;
	_nop_();
	SCL = 1;
	_nop_();
	SDA = 1;
}

/**
 * @description: 读取按键
 * @return {uchar} key
 */
uchar keyTM1637()
{
	uchar key = 0, i = 0, temp = 0;
	startTM1637();
	writeTM1637(0x42);
	ackTM1637();
	SDA = 1;
	for (; i < 8; i++)
	{
		temp >>= 1;
		_nop_();
		SCL = 1;
		if (SDA)
			temp |= 0x80;
		_nop_();
		SCL = 0;
		_nop_();
	}
	ackTM1637();
	stopTM1637();

	switch (temp)
	{
	case 0xFF:
		key = 0;
		break;
	case 0xF7:
		key = 1;
		break;
	case 0xF6:
		key = 2;
		break;
	case 0xF5:
		key = 3;
		break;
	case 0xF4:
		key = 4;
		break;
	case 0xF3:
		key = 5;
		break;
	default:
		key = 6;
		break;
	}
	return key;
}

/**
 * @description: 设置亮度
 * @param {uchar} luminance 亮度
 * @return {*}
 */
void setluminanceTM1637(uchar luminance)
{
	uchar temp = 0;
	if (luminance)
		temp = (luminance - 1) | 0x88;
	else
		temp = 0x80;

	startTM1637();
	writeTM1637(temp);
	ackTM1637();
	stopTM1637();
}

/**
 * @description: 数码管显示
 * @param {uchar} grid 位置
 * @param {uchar} sg   显示码
 * @return {*}
 */
void displayTM1637(uchar grid, uchar sg)
{
	startTM1637();
	writeTM1637(0x44);
	ackTM1637();
	stopTM1637();

	startTM1637();
	writeTM1637((grid - 1) | 0xC0);
	ackTM1637();
	writeTM1637(sg);
	ackTM1637();
	stopTM1637();
}
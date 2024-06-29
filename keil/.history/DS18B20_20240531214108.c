#include "DS18B20.H"

// 12MHz
void delay5us(uchar n)
{
	do
	{
		_nop_();
		_nop_();
		_nop_();
		n--;
	} while (n);
}

/**
 * @description: 初始化
 * @return {bit} ack 0应答，1无应答
 */
bit initDS18B20()
{
	bit ack = 0;
	DQ = 0;
	delay5us(96);
	DQ = 1;
	delay5us(12);
	ack = DQ;
	delay5us(60); // 太大太小都不行
	return ack;
}

/**
 * @description: 向DS18B20读取一字节数据
 * @return {uchar}
 */
uchar readbyteDS18B20()
{
	uchar i = 0;
	uchar dat = 0;
	for (i = 0; i < 8; i++)
	{
		DQ = 0;
		_nop_();
		_nop_();
		DQ = 1;
		dat >>= 1;
		if (DQ)
		{
			dat |= 0x80;
		}
		delay5us(11);
	}
	return dat;
}

/**
 * @description: 向DS18B20写入一字节数据，低位在前
 * @param {uchar} dat
 * @return {*}
 */
void writebyteDS18B20(uchar dat)
{
	uchar i = 0;
	for (i = 0; i < 8; i++)
	{
		DQ = 0;
		_nop_();
		_nop_();
		DQ = dat & 0x01;
		delay5us(12);
		DQ = 1;
		dat >>= 1;
		_nop_();
		_nop_();
	}
}

/********************************************************/
/*DS18B20的CRC8校验程序*/
/********************************************************/
/**
 * @description: 一字节CRC8
 * @param {uchar} abyte
 * @return {uchar} crc
 */
uchar calcrc_1byte(uchar abyte)
{
	uchar i, crc_1byte;
	crc_1byte = 0; // 设定crc_1byte初值为0
	for (i = 0; i < 8; i++)
	{
		if ((crc_1byte ^ abyte) & 0x01)
		{
			crc_1byte ^= 0x18;
			crc_1byte >>= 1;
			crc_1byte |= 0x80;
		}
		else
			crc_1byte >>= 1;
		abyte >>= 1;
	}
	return crc_1byte;
}
/**
 * @description: 多字节CRC8
 * @param {uchar} *p
 * @param {uchar} len
 * @return {uchar} crc
 */
uchar calcrc_bytes(uchar *p, uchar len)
{
	uchar crc = 0;
	while (len--) // len为总共要校验的字节数
	{
		crc = calcrc_1byte(crc ^ *p++);
	}
	return crc;
}

/**
 * @description: 读取温度，如果数据传输不正确则报警间隔1s
 * @return {uint} temp
 */
uint Read_temp_DS18B20()
{
	uchar i = 0;
	uchar byte9[9];
	uint temp = 0;
	initDS18B20();
	writebyteDS18B20(Skip_Rom);
	writebyteDS18B20(Convert_T);
	delay1ms(100);
	initDS18B20();
	writebyteDS18B20(Skip_Rom);
	writebyteDS18B20(Read_Scratchpad);
	for (i = 0; i < 9; i++)
	{
		byte9[i] = readbyteDS18B20();
	}
	while (calcrc_bytes(&byte9, 9)) // 数据正确CRC应为0
	{
		buzzer = 0;
		delay1ms(1000);
		buzzer = 1;
		delay1ms(1000);
	}
	temp = byte9[0] | ((byte9[1] | temp) << 8);
	return temp;
}

/**
 * @description: 读取EEPROM中的上下限温度，高8位为上限，低8位为下限，如果数据传输不正确则警报间隔2s
 * @return {uint} temp
 */
uint Read_EEPROM_DS18B20()
{
	uchar i = 0;
	uchar byte9[9];
	uint temp = 0;
	initDS18B20();
	writebyteDS18B20(Skip_Rom);
	writebyteDS18B20(Recall_EEPROM);
	initDS18B20();
	writebyteDS18B20(Skip_Rom);
	writebyteDS18B20(Read_Scratchpad);
	for (i = 0; i < 9; i++)
	{
		byte9[i] = readbyteDS18B20();
	}
	while (calcrc_bytes(&byte9, 9)) // 数据正确CRC应为0
	{
		buzzer = 0;
		delay1ms(2000);
		buzzer = 1;
		delay1ms(2000);
	}
	temp = byte9[3] | ((byte9[2] | temp) << 8);
	return temp;
}

/**
 * @description: 向暂存器写入数据并存入EERROM
 * @param {uchar} TL:下限
 * @param {uchar} precision:转换精度
 * @return {*}
 */
void Write_Scratchpad_EEPROM_DS18B20(uchar TH, uchar TL, uchar precision)
{
	initDS18B20();
	writebyteDS18B20(Skip_Rom);
	writebyteDS18B20(Write_Scratchpad);
	writebyteDS18B20(TH);
	writebyteDS18B20(TL);
	writebyteDS18B20(precision);
	initDS18B20();
	writebyteDS18B20(Skip_Rom);
	writebyteDS18B20(Copy_Scratchpad);
}
#include "TM1637.h"

uchar temp = 0;					// 缓存
uint timer = 0;					// 时间计次
uchar luminance = 8;			// 默认亮度为8挡，最亮
uchar precision = precision_12; // 默认转换精度12位
uchar keynum = 0;				// 按键
int original = 0;				// 原始温度数据，16位补码，高5位符号位，低11数据位
uchar tens_unit = 0;			// 十分位
uchar unit = 0;					// 个位
uchar tens_digit = 0;			// 十位
char TL = 0, TH = 0;			// 温度上下限
char tempreatuer = 0;			// 用于比较温度上下限的只有个位和十位的带符号温度数据

/*共阴段码		0	  1	    2	  3	    4	  5	    6	  7	    8	  9	    C	  负号  灭  全亮  */
uchar sg[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x39, 0x40, 0, 0xFF};
uchar sg1[] = {0xBF, 0x86, 0xDB, 0xCF, 0xE6, 0xED, 0xFD, 0x87, 0xFF, 0xEF}; // 带点共阴段码0. 1. 2. 3. 4. 5. 6. 7. 8. 9.

void timer0_interrupt_init();
uchar timer0_50ms();
void displaytemp();
char set_limit(char TH_or_TL);
void setluminance();

void main()
{
	uchar i = 0, j = 0;

	// 开局一转换，需约750ms
	initDS18B20();
	writebyteDS18B20(Skip_Rom);
	writebyteDS18B20(Convert_T);

	// 开局一读取，获取DS18B20的EEPROM中储存的温度上下限
	TH = Read_EEPROM_DS18B20() >> 8;
	TL = Read_EEPROM_DS18B20();

	// 开局一设置，设置亮度
	setluminanceTM1637(luminance);
	for (i = 0; i < 4; i++) // 等待温度转换，顺便测试数码管是否有损坏
	{
		for (j = 0; j < 4; j++)
			displayTM1637(j + 1, sg[12]);
		displayTM1637(i + 1, sg[13]);
		delay1ms(200);
	}

	// 若DS18B20正常通讯
	while (!initDS18B20())
	{
		keynum = keyTM1637();
		timer = timer0_50ms();
		switch (keynum)
		{
		case 1:
			/*--------------------------------------------------
									调节亮度
						长按K1两秒以上进入亮度调节模式
						  进入此模式后各按键功能如下：
					K1:+1	K2:-1	K3:+2	K4:-2	K5:设置结束
								范围：5~8
			--------------------------------------------------*/
			if (timer >= 40)
				setluminance();
			/*--------------------------------------------------
								设置温度上限
						短按K1两秒以内进入温度上限设置模式
						  进入此模式后各按键功能如下：
				K1:转为正数，设置范围：0~99
				K2:转为负数，设置范围：-49~0
				K3:十位循环加一，范围（0不显示）：负数是0~4；正数是0~9
				K4:个位循环加一，范围：0~9
				K5:设置结束，将数据存入DS18B20的EEPROM中
			--------------------------------------------------*/
			else
			{
				TH = Read_EEPROM_DS18B20() >> 8;
				TH = set_limit(TH);
				Write_Scratchpad_EEPROM_DS18B20(TH, TL, precision);
			}
			timer = 0;
			break;
			/*--------------------------------------------------
								设置温度下限
							按K2进入温度下限设置模式
							进入此模式后各按键功能如下：
				K1:转为正数，设置范围：0~99
				K2:转为负数，设置范围：-49~0
				K3:十位循环加一，范围（0不显示）：负数是0~4；正数是0~9
				K4:个位循环加一，范围：0~9
				K5:设置结束，将数据存入DS18B20的EEPROM中
			--------------------------------------------------*/
		case 2:
			TL = Read_EEPROM_DS18B20();
			TL = set_limit(TL);
			Write_Scratchpad_EEPROM_DS18B20(TH, TL, precision);
			break;
		default:
			displaytemp();
			break;
		}
		// 如果温度低于下限或者高于上限，警报一直响
		if (tempreatuer >= TH || tempreatuer <= TL)
		{
			buzzer = 0;
			delay1ms(100);
		}
		else
			buzzer = 1;
	}
	// 若无法与DS18B20通讯，则蜂鸣器响5s后关闭
	buzzer = 0;
	delay1ms(5000);
}

/**
 * @description: 50ms定时器中断初始化@12.000MHz
 * @return {*}
 */
void timer0_interrupt_init()
{
	EA = 1;
	ET0 = 1;
	TMOD &= 0xF0; // 设置定时器模式
	TMOD |= 0x01; // 设置定时器模式
	TL0 = 0xB0;	  // 设置定时初始值
	TH0 = 0x3C;	  // 设置定时初始值
	TF0 = 0;	  // 清除TF0标志
	TR0 = 1;	  // 定时器0开始计时
}

/**
 * @description: 按键按下计时，timer*50ms@12.000MHz
 * @return {uchar} timer
 */
uchar timer0_50ms()
{
	ET0 = 0;	  // 关闭中断，以免中断与TM637时序冲突
	TMOD &= 0xF0; // 设置定时器模式
	TMOD |= 0x01; // 设置定时器模式
	timer = 0;
	while (keyTM1637())
	{
		TL0 = 0xB0; // 设置定时初始值
		TH0 = 0x3C; // 设置定时初始值
		TF0 = 0;	// 清除TF0标志
		TR0 = 1;	// 定时器0开始计时
		do
		{
		} while (!TF0);
		timer++;
	}
	TR0 = 0; // 停止计时
	return timer;
}

/**
 * @description: 数码管显示温度
 * @return {*}
 */
void displaytemp()
{
	original = Read_temp_DS18B20();
	if (original < 0) // 负
	{
		temp = (~original + 1) >> 4;
		unit = temp % 10;							 // 个位
		tens_digit = temp / 10 % 10;				 // 十位
		tempreatuer = ~(tens_digit * 10 + unit) + 1; // 负数原码取反加一得补码
		displayTM1637(3, sg[unit]);
		if (tens_digit)
		{
			displayTM1637(2, sg[tens_digit]);
			displayTM1637(1, sg[11]);
		}
		else
		{
			displayTM1637(2, sg[11]);
			displayTM1637(1, sg[12]);
		}
	}
	else // 正
	{
		tens_unit = (0x000F & original) * 625 / 1000 % 10; // 十分位
		unit = original / 16 % 10;						   // 个位
		tens_digit = original / 160 % 10;				   // 十位
		tempreatuer = tens_digit * 10 + unit;			   // 正数原码补码一致
		displayTM1637(3, sg[tens_unit]);
		displayTM1637(2, sg1[unit]);
		if (tens_digit)
			displayTM1637(1, sg[tens_digit]);
		else
			displayTM1637(1, sg[12]);
	}
	displayTM1637(4, sg[10]); // 单位摄氏度C
	delay1ms(5);			  // 过一会儿再更新温度，否则数码管闪烁,太快了时序会错，暂时不知为何
}

/**
 * @description: 设置阈值
 * @param {char} TH_or_TL
 * @return {char} TH_or_TL
 */
char set_limit(char TH_or_TL)
{
	bit mode1 = 0; // 符号位，1为负数，0为正数
	displayTM1637(4, sg[10]);
	if (TH_or_TL < 0)
	{
		mode1 = 1;
		/*在这里不用temp会错误，不知为何*/
		temp = ~TH_or_TL + 1;
		unit = temp % 10;
		tens_digit = temp / 10 % 10;
	}
	else
	{
		mode1 = 0;
		unit = TH_or_TL % 10;
		tens_digit = TH_or_TL / 10 % 10;
	}
	while (keynum != 5)
	{
		if (mode1)
		{
			if (tens_digit > 4)
				tens_digit = 4;
			keynum = keyTM1637();
			while ((keynum != 1) && (keynum != 5))
			{
				if (tens_digit)
				{
					displayTM1637(1, sg[11]);
					displayTM1637(2, sg[tens_digit]);
					displayTM1637(3, sg[unit]);
				}
				else
				{
					displayTM1637(1, sg[12]);
					displayTM1637(2, sg[11]);
					displayTM1637(3, sg[unit]);
				}
				delay1ms(2);
				keynum = keyTM1637();
				while (keyTM1637())
					delay1ms(2);
				if (keynum == 3)
				{
					if (tens_digit == 4)
						tens_digit = 0;
					else
						tens_digit++;
				}
				if (keynum == 4)
				{
					if (unit == 9)
						unit = 0;
					else
						unit++;
				}
			}
			if (keynum == 1)
				mode1 = 0;
		}
		if (!mode1)
		{
			while ((keynum != 2) && (keynum != 5))
			{
				if (tens_digit)
				{
					displayTM1637(2, sg[tens_digit]);
					displayTM1637(3, sg[unit]);
				}
				else
				{
					displayTM1637(2, sg[12]);
					displayTM1637(3, sg[unit]);
				}
				displayTM1637(1, sg[12]);
				delay1ms(2);
				keynum = keyTM1637();
				while (keyTM1637())
					delay1ms(2);
				if (keynum == 3)
				{
					if (tens_digit == 9)
						tens_digit = 0;
					else
						tens_digit++;
				}
				if (keynum == 4)
				{
					if (unit == 9)
						unit = 0;
					else
						unit++;
				}
			}
			if (keynum == 2)
				mode1 = 1;
		}
	}
	TH_or_TL = tens_digit * 10 + unit;
	if (mode1)
		TH_or_TL = ~TH_or_TL + 1; // 转补码
	return TH_or_TL;
}

void setluminance()
{
	displayTM1637(1, sg[12]);
	displayTM1637(2, sg[12]);
	displayTM1637(3, sg[12]);
	timer = 0;
	timer0_interrupt_init();
	while (keynum != 5)
		;
	setluminanceTM1637(luminance);
}

#ifndef __VSCODE_C51__
#define INTERRUPT(x) interrupt x
#else
#define INTERRUPT(x)
#endif

/**
 * @description: 利用中断函数实现要更改的数字间隔闪烁
 * @return {*}
 */
void interrupt0_setluminance() INTERRUPT(1)
{
	static bit mode = 0;
	TR0 = 0;	// 进入中断函数关闭计时
	TL0 = 0xB0; // 设置定时初始值
	TH0 = 0x3C; // 设置定时初始值
	timer++;
	if (timer >= 10) // 0.5s闪烁
	{
		timer = 0;
		if (mode)
			displayTM1637(4, sg[luminance]);
		else
			displayTM1637(4, sg[12]);
		mode = ~mode;
	}
	keynum = keyTM1637();
	while (keyTM1637())
		delay1ms(2); // 循环太快TM1637一直在读取按键，影响数码管显示，会闪烁
	switch (keynum)
	{
	case 1:
		if (luminance == 8)
			luminance = 5;
		else
			luminance++;
		break;
	case 2:
		if (luminance == 5)
			luminance = 8;
		else
			luminance--;
		break;
	case 3:
		switch (luminance)
		{
		case 7:
			luminance = 5;
			break;
		case 8:
			luminance = 6;
			break;
		default:
			luminance += 2;
			;
			break;
		}
		break;
	case 4:
		switch (luminance)
		{
		case 5:
			luminance = 7;
			break;
		case 6:
			luminance = 8;
			break;
		default:
			luminance -= 2;
			;
			break;
		}
		break;
	default:
		break;
	}
	TR0 = 1; // 开始计时
}
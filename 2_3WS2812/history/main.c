#include "stc8h.h"
#include "intrins.h"
#include "string.h"
#define MAIN_Fosc 22118400L // 定义主时钟（精确计算115200波特率）

//========================================================================
//                               类型定义
//========================================================================
typedef unsigned char u8;	  //  8 bits
typedef unsigned int u16;	  // 16 bits
typedef unsigned long u32;	  // 32 bits
typedef signed char int8;	  //  8 bits
typedef signed int int16;	  // 16 bits
typedef signed long int32;	  // 32 bits
typedef unsigned char uint8;  //  8 bits
typedef unsigned int uint16;  // 16 bits
typedef unsigned long uint32; // 32 bits
#define TRUE 1
#define FALSE 0
#define NULL 0

//===================================================
//						Uart1相关定义
//===================================================
#define Baudrate1 115200L
#define UART1_BUF_LENGTH 4
u8 TX1_Cnt;							   // 发送计数
u8 RX1_Cnt;							   // 接收计数
bit B_TX1_Busy;						   // 发送忙标志
u8 xdata RX1_Buffer[UART1_BUF_LENGTH]; // 接收缓冲
void UART1_config(u8 brt);			   // 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer1做波特率.
void PrintString1(u8 *puts);

//===========================================================
//                  WS2812定义
//==========================================================
#define RGB_PIN P54
#define RGB_NUMLEDS 2 // 个数

//=====================================================
//                        颜色缓存
//=========================================================
xdata uint8 _rgb_buf_R[RGB_NUMLEDS] = {0};
xdata uint8 _rgb_buf_G[RGB_NUMLEDS] = {0};
xdata uint8 _rgb_buf_B[RGB_NUMLEDS] = {0};

// IO_Init
void IO_Init();

// 延迟函数@24.000MHz
void Delay1us(void);
void Delay50us(void);
void Delay1ms(void);
void Delay20ms(void);
void Delay200ms(void);
void Delay500ms(void);
void Delay_ms(uint16 time);

// WS2812函数
void rgb_init();
void rgb_set_up();
void rgb_set_down();
void rgb_write(uint8 G8, uint8 R8, uint8 B8);
void rgb_rst();
void rgb_set_color(uint8 num, uint8 r, uint8 g, uint8 b);
void rgb_hal_delay(unsigned int t);
void rgb_show(uint8 num, uint8 r, uint8 g, uint8 b); // 默认直接调用该函数

// 三色LED
void Shutdown_LED();
void Ctrl_LED(unsigned char *rxdata);

// Uart
void PrintString1(u8 *puts);
void SetTimer2Baudraye(u16 dat);
void UART1_config(u8 brt);

//=========================================================
//						主函数
//=========================================================
void main()
{
	IO_Init();
	rgb_init();

	UART1_config(1); // 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer1做波特率.
	EA = 1;			 // 允许总中断

	PrintString1("STC8H8K64U UART1 Test Programme!\r\n"); // UART1发送一个字符串

	rgb_show(1, 1, 1, 128);
	Delay_ms(200);
	rgb_show(1, 1, 1, 1);
	Delay_ms(200);

	while (1)
	{
		char RX_Data[UART1_BUF_LENGTH]; // 存储接收的数据
		unsigned char i;

		//rgb_show(1, 1, 1, 1);
		//rgb_show(1, 1, 1, 128);
		if ((TX1_Cnt != RX1_Cnt) && (!B_TX1_Busy)) // 收到数据, 发送空闲
		{
			// SBUF = RX1_Buffer[TX1_Cnt]; // 把收到的数据远样返回
			for (i = 0; i < 4; i++)
			{
				RX_Data[i] = RX1_Buffer[i]; // 复制数据
			}
			RX_Data[4] = '\0';
			Ctrl_LED(RX_Data);
			SBUF = *RX_Data;
			B_TX1_Busy = 1;
			if (++TX1_Cnt >= UART1_BUF_LENGTH)
				TX1_Cnt = 0;
		}
	}
}

//==================================================
//					void IO_Init();
//==================================================
void IO_Init()
{
	P_SW2 |= 0x80; // 扩展寄存器(XFR)访问使能
	P0M1 = 0x00;
	P0M0 = 0x00;
	P1M1 = 0x00;
	P1M0 = 0x00;
	P2M1 = 0x00;
	P2M0 = 0x00;
	P3M1 = 0x00;
	P3M0 = 0x00;
	P4M1 = 0x00;
	P4M0 = 0x00;
	P5M1 = 0x00;
	P5M0 = 0x00;
	P6M1 = 0x00;
	P6M0 = 0x00;
	P7M1 = 0x00;
	P7M0 = 0x00;
}

//=====================================================
//                    delay
//===========================================
void Delay1us() //@24.000MHz
{
	uint8 i;

	i = 6;
	while (--i)
		;
}

void Delay50us() //@24.000MHz
{
	uint8 i, j;

	_nop_();
	i = 2;
	j = 140;
	do
	{
		while (--j)
			;
	} while (--i);
}

void Delay1ms(void) //@24.000MHz
{
	unsigned char data i, j;

	_nop_();
	i = 32;
	j = 40;
	do
	{
		while (--j)
			;
	} while (--i);
}

void Delay20ms(void) //@24.000MHz
{
	unsigned char data i, j, k;

	_nop_();
	i = 3;
	j = 112;
	k = 91;
	do
	{
		do
		{
			while (--k)
				;
		} while (--j);
	} while (--i);
}

void Delay_ms(uint16 time)
{
	if (time < 1)
	{
		time = 1;
	}
	do
	{
		Delay1ms();
	} while (--time);
}

//========================================================================
// 描述: 全彩LED初始化，延迟5微秒.
// 参数: none.
// 返回: none.
//========================================================================
void rgb_init()
{
	P5M0 = 0x00;
	P5M1 = 0x00;
}

//========================================================================
// 描述: 1码，高电平850ns 低电平400ns 误差正负150ns.
// 参数: none.
// 返回: none.
//========================================================================
void rgb_set_up()
{
	RGB_PIN = 1;
	// 经过逻辑分析仪调试的的延时
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	RGB_PIN = 0;
}

//========================================================================
// 描述: 0码，高电平400ns 低电平850ns 误差正负150ns.
// 参数: none.
// 返回: none.
//========================================================================
void rgb_set_down()
{
	RGB_PIN = 1;
	// 经过逻辑分析仪调试的的延时
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	RGB_PIN = 0;
}

//========================================================================
// 描述: 发送24位数据.
// 参数: 绿色8位，红色8位，蓝色8位.
// 返回: none.
//========================================================================
void rgb_write(uint8 G8, uint8 R8, uint8 B8)
{
	unsigned int n = 0;
	// 发送G8位
	EA = 0;
	for (n = 0; n < 8; n++)
	{

		if ((G8 & 0x80) == 0x80)
		{
			rgb_set_up();
		}
		else
		{
			rgb_set_down();
		}
		G8 <<= 1;
	}
	// 发送R8位
	for (n = 0; n < 8; n++)
	{

		if ((R8 & 0x80) == 0x80)
		{
			rgb_set_up();
		}
		else
		{
			rgb_set_down();
		}
		R8 <<= 1;
	}
	// 发送B8位
	for (n = 0; n < 8; n++)
	{

		if ((B8 & 0x80) == 0x80)
		{
			rgb_set_up();
		}
		else
		{
			rgb_set_down();
		}
		B8 <<= 1;
	}
	EA = 1;
}

//========================================================================
// 描述: 复位.
// 参数: none.
// 返回: none.
//========================================================================
void rgb_rst()
{
	RGB_PIN = 0;
	delay50us();
}

// //========================================================================
// // 描述: 把24位数据GRB码转RGB.
// // 参数: none.
// // 返回: none.
// //========================================================================
// void Set_Colour(uint8 r, uint8 g, uint8 b)
// {
// 	uint8 i;
// 	for (i = 0; i < RGB_NUMLEDS; i++)
// 	{
// 		_rgb_buf_R[i] = r; //缓冲
// 		_rgb_buf_G[i] = g;
// 		_rgb_buf_B[i] = b;
// 	}
// 	for (i = 0; i < RGB_NUMLEDS; i++)
// 	{
// 		rgb_write(_rgb_buf_G[i], _rgb_buf_R[i], _rgb_buf_B[i]);//发送显示
// 	}
// }

//========================================================================
// 描述: 设置指定点的显示颜色.
// 参数: 第几个RGB灯，R值,G值,B值.
// 返回: none.
//========================================================================
void rgb_set_color(uint8 num, uint8 r, uint8 g, uint8 b)
{
	uint8 i;
	for (i = 0; i < RGB_NUMLEDS; i++)
	{
		_rgb_buf_R[num] = r; // 缓冲
		_rgb_buf_G[num] = g;
		_rgb_buf_B[num] = b;
	}
	for (i = 0; i < RGB_NUMLEDS; i++)
	{
		rgb_write(_rgb_buf_G[i], _rgb_buf_R[i], _rgb_buf_B[i]); // 发送显示
	}
}

// //========================================================================
// // 描述: 颜色交换24位不拆分发.
// // 参数: none.
// // 返回: none.
// //========================================================================
// void SetPixelColor(uint8 num, uint32 c)
// {
// 	uint8 i;
// 	for (i = 0; i < RGB_NUMLEDS; i++)
// 	{
// 		_rgb_buf_R[num] = (uint8)(c >> 16);
// 		_rgb_buf_G[num] = (uint8)(c >> 8);
// 		_rgb_buf_B[num] = (uint8)(c);
// 	}
// 	for (i = 0; i < RGB_NUMLEDS; i++)
// 	{
// 		rgb_write(_rgb_buf_G[i], _rgb_buf_R[i], _rgb_buf_B[i]);
// 	}
// }

//========================================================================
// 描述: rgb延时.
// 参数: none.
// 返回: none.
//========================================================================
void rgb_hal_delay(unsigned int t)
{
	unsigned int x, y;
	for (x = 114; x > 0; x--)
		for (y = t; y > 0; y--)
			;
}

//========================================================================
// 描述: 延迟5微秒.
// 参数: none.
// 返回: none.
//========================================================================
void rgb_show(uint8 num, uint8 r, uint8 g, uint8 b)
{
	rgb_rst();
	rgb_hal_delay(30);
	rgb_set_color(num, r, g, b);
}

//=================================================
//					三色LED
//==================================================
// 关闭三色LED
void Shutdown_LED()
{
	P00 = 1;
	P01 = 1;
	P02 = 1;
}

// 串口控制三色LED
void Ctrl_LED(unsigned char *rxdata)
{
	Shutdown_LED();

	// 使用strcmp比较字符串
	if (strcmp(rxdata, "LEDA") == 0)
	{
		P00 = 0;
		P01 = 0;
		P02 = 0;
	}
	else if (strcmp(rxdata, "SHUT") == 0)
	{
		Shutdown_LED();
	}
	else if (strcmp(rxdata, "LEDB") == 0)
	{
		P00 = 0;
		P01 = 1;
		P02 = 1;
	}
	else if (strcmp(rxdata, "LEDR") == 0)
	{
		P00 = 1;
		P01 = 0;
		P02 = 1;
	}
	else if (strcmp(rxdata, "LEDG") == 0)
	{
		P00 = 1;
		P01 = 1;
		P02 = 0;
	}
	else if (strcmp(rxdata, "RGBR") == 0)
	{
		P00 = 0;
		Delay500ms();
		P00 = 1;
		P01 = 0;
		Delay500ms();
		P01 = 1;
		P02 = 0;
		Delay500ms();
		P02 = 1;
	}
}

//========================================================================
// 函数: void PrintString1(u8 *puts)
// 描述: 串口1发送字符串函数。
// 参数: puts:  字符串指针.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注:
//========================================================================
void PrintString1(u8 *puts) // 发送一个字符串
{
	for (; *puts != 0; puts++) // 遇到停止符0结束
	{
		SBUF = *puts;
		B_TX1_Busy = 1;
		while (B_TX1_Busy)
			;
	}
}

//========================================================================
// 函数: SetTimer2Baudraye(u16 dat)
// 描述: 设置Timer2做波特率发生器。
// 参数: dat: Timer2的重装值.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注:
//========================================================================
void SetTimer2Baudraye(u16 dat) // 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer1做波特率.
{
	AUXR &= ~(1 << 4); // Timer stop
	AUXR &= ~(1 << 3); // Timer2 set As Timer
	AUXR |= (1 << 2);  // Timer2 set as 1T mode
	T2H = dat / 256;
	T2L = dat % 256;
	IE2 &= ~(1 << 2); // 禁止中断
	AUXR |= (1 << 4); // Timer run enable
}

//========================================================================
// 函数: void UART1_config(u8 brt)
// 描述: UART1初始化函数。
// 参数: brt: 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer1做波特率.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注:
//========================================================================
void UART1_config(u8 brt) // 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer1做波特率.
{
	/*********** 波特率使用定时器2 *****************/
	if (brt == 2)
	{
		AUXR |= 0x01; // S1 BRT Use Timer2;
		SetTimer2Baudraye(65536UL - (MAIN_Fosc / 4) / Baudrate1);
	}

	/*********** 波特率使用定时器1 *****************/
	else
	{
		TR1 = 0;
		AUXR &= ~0x01;	   // S1 BRT Use Timer1;
		AUXR |= (1 << 6);  // Timer1 set as 1T mode
		TMOD &= ~(1 << 6); // Timer1 set As Timer
		TMOD &= ~0x30;	   // Timer1_16bitAutoReload;
		TH1 = (u8)((65536UL - (MAIN_Fosc / 4) / Baudrate1) / 256);
		TL1 = (u8)((65536UL - (MAIN_Fosc / 4) / Baudrate1) % 256);
		ET1 = 0;		  // 禁止中断
		INTCLKO &= ~0x02; // 不输出时钟
		TR1 = 1;
	}
	/*************************************************/

	SCON = (SCON & 0x3f) | 0x40; // UART1模式, 0x00: 同步移位输出, 0x40: 8位数据,可变波特率, 0x80: 9位数据,固定波特率, 0xc0: 9位数据,可变波特率
	//  PS  = 1;    //高优先级中断
	ES = 1;	 // 允许中断
	REN = 1; // 允许接收
	P_SW1 &= 0x3f;
	P_SW1 |= 0x00; // UART1 switch to, 0x00: P3.0 P3.1, 0x40: P3.6 P3.7, 0x80: P1.6 P1.7, 0xC0: P4.3 P4.4
	//  PCON2 |=  (1<<4);   //内部短路RXD与TXD, 做中继, ENABLE,DISABLE

	B_TX1_Busy = 0;
	TX1_Cnt = 0;
	RX1_Cnt = 0;
}

//========================================================================
// 函数: void UART1_int (void) interrupt UART1_VECTOR
// 描述: UART1中断函数。
// 参数: nine.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注:
//========================================================================
void UART1_int(void) interrupt 4
{
	if (RI)
	{
		RI = 0;
		RX1_Buffer[RX1_Cnt] = SBUF;
		if (++RX1_Cnt >= UART1_BUF_LENGTH)
			RX1_Cnt = 0; // 防溢出
	}

	if (TI)
	{
		TI = 0;
		B_TX1_Busy = 0;
	}
}

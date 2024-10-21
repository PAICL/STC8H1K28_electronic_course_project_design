/*===============================================================
����Ŀ����STC8H1K28�Լ��ٷ����������п���

����ĿΪͨ�����ڿ��ƿ�������LED��ɫ���Լ�����WS2812�ʵ�
	��ִ��ָ�
	LEDA#	����ɫLED
	SHUT#	�ر����е�
	LEDB#	����ɫLED������
	LEDR#	����ɫLED�ĺ��
	LEDG#	����ɫLED���̵�
	RALL#	������WS2812
	RAO1#	�򿪵�һ��WS2812��R:RGB, A:Null, O:Open, 1:��һ���ƣ���ͬ��
	RAO2#	�򿪵ڶ���WS2812
	RAC1#	�رյ�һ��WS2812��R:RGB, A:Null, C:Cancel, 1:��һ���ƣ���ͬ��
	RAC2#	�رյ�һ��WS2812
	RGB1#	ͬRAO1#
	RGB2#	ͬRAO2#
	RGBR#	�򿪵�һ��WS2812��ƣ���д��ʾ��һ���ƣ�Сд��ʾ�ڶ����ƣ�
	RGBG#	�򿪵�һ��WS2812�̵ƣ�R:Red, G��Green, B:Blue��
	RGBB#	�򿪵�һ��WS2812����
	RGBr#	�򿪵ڶ���WS2812���
	RGBg#	�򿪵ڶ���WS2812���
	RGBb#	�򿪵ڶ���WS2812���

������¼ʱ��Ҫ��Ƶ������Ϊ22.1184MHz�������޷�����
Warning����ԭ����������Ĵ��뺯��δʹ�ã���Ӱ��������ɹ�
����������޷����룬�����Warningԭ������ɾ������Ĺ��ܺ���
Data:2024/10/20
*///===============================================================

#include "stc8h.h"
#include "intrins.h"
#include "string.h"
#define MAIN_Fosc 22118400L // ������ʱ�ӣ���ȷ����115200�����ʣ�
// #define MAIN_Fosc 24000000L // ������ʱ�ӣ���ȷ����115200�����ʣ�

//========================================================================
//                               ���Ͷ���
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
//						Uart1��ض���
//===================================================
#define Baudrate1 115200L
#define UART1_BUF_LENGTH 8
bit receiving_complete = 0;
u8 TX1_Cnt;							   // ���ͼ���
u8 RX1_Cnt;							   // ���ռ���
bit B_TX1_Busy;						   // ����æ��־
u8 xdata RX1_Buffer[UART1_BUF_LENGTH]; // ���ջ���
void UART1_config(u8 brt);			   // ѡ������, 2: ʹ��Timer2��������, ����ֵ: ʹ��Timer1��������.
void PrintString1(u8 *puts);

//===========================================================
//                  WS2812����
//==========================================================
#define RGB_PIN P54
#define RGB_NUMLEDS 2 // ����

//=====================================================
//                        ��ɫ����
//=========================================================
xdata uint8 _rgb_buf_R[RGB_NUMLEDS] = {0};
xdata uint8 _rgb_buf_G[RGB_NUMLEDS] = {0};
xdata uint8 _rgb_buf_B[RGB_NUMLEDS] = {0};

// IO_Init
void IO_Init();

// �ӳٺ���@24.000MHz
void Delay1us(void);
void Delay50us(void);
void Delay1ms(void);
void Delay20ms(void);
void Delay200ms(void);
void Delay500ms(void);
void Delay_ms(uint16 time);

// WS2812����
void rgb_init();
void rgb_set_up();
void rgb_set_down();
void rgb_write(uint8 G8, uint8 R8, uint8 B8);
void rgb_rst();
void rgb_set_color(uint8 num, uint8 r, uint8 g, uint8 b);
void rgb_hal_delay(unsigned int t);
void rgb_show(uint8 num, uint8 r, uint8 g, uint8 b); // Ĭ��ֱ�ӵ��øú���

// ��ɫLED
void Shutdown_LED();
void Ctrl_LED(unsigned char *rxdata);

// Uart
void PrintString1(u8 *puts);
void SetTimer2Baudraye(u16 dat);
void UART1_config(u8 brt);

enum UART_State
{
	UART_IDLE,
	UART_RECEIVE,
	UART_SEND
};

//=========================================================
//						������
//=========================================================
void main()
{
	// ����������ݻ�����
	char RX_Data[UART1_BUF_LENGTH];			  // �洢���յ�����
	unsigned char i;						  // ѭ��������
	enum UART_State currentState = UART_IDLE; // ��ʼ����ǰ״̬Ϊ����״̬

	// ��ʼ��I/O�ں�RGB LED
	IO_Init();
	rgb_init();

	// ����UART1������
	UART1_config(1); // ѡ������, 2: ʹ��Timer2��������, ����ֵ: ʹ��Timer1��������.
	EA = 1;			 // ����ȫ���ж�

	// ���ͳ�ʼ���ַ�����UART1
	PrintString1("STC8H8K64U UART1, Development by Leyue Chen\r\n"); // UART1����һ���ַ���

	// RGB LED��˸��ָʾ��ʼ�����
	rgb_show(2, 0, 0, 128);
	Delay_ms(200);
	rgb_show(2, 0, 0, 0);
	Delay_ms(200);

	// ��ѭ��
	while (1)
	{
		char RX_Data[UART1_BUF_LENGTH]; // ??????
		unsigned char i;

		if (receiving_complete) // ??????
		{
			for (i = 0; i < RX1_Cnt; i++)
			{
				RX_Data[i] = RX1_Buffer[i]; // ????
			}
			RX_Data[RX1_Cnt] = '\0'; // ????????

			Ctrl_LED(RX_Data);

			// ??????????
			RX1_Cnt = 0;
			receiving_complete = 0; // ????????
		}

		/*
		switch (currentState)
		{
		case UART_IDLE: // ����״̬
			// ����Ƿ������ݽ��գ����ҵ�ǰ��æ�ڷ���
			if ((TX1_Cnt != RX1_Cnt) && (!B_TX1_Busy))
			{
				// �л�������״̬
				currentState = UART_RECEIVE;
			}
			else
			{
				// ������������
			}
			break;

		case UART_RECEIVE: // ����״̬
			// �ӽ��ջ�������������
			for (i = 0; i < UART1_BUF_LENGTH; i++)
			{
				RX_Data[i] = RX1_Buffer[i]; // ��������
			}
			RX_Data[UART1_BUF_LENGTH] = '\0'; // ȷ���ַ���������
			Ctrl_LED(RX_Data);				  // ����LED״̬
			currentState = UART_SEND;		  // �л�������״̬
			break;

		case UART_SEND: // ����״̬
			// ���ͽ��յ��ĵ�һ���ֽ�
			SBUF = RX_Data[0];				   // ���͵�һ���ֽ�
			B_TX1_Busy = 1;					   // ���÷���æ��־
			if (++TX1_Cnt >= UART1_BUF_LENGTH) // ���·��ͼ�����
			{
				TX1_Cnt = 0; // ѭ��������
			}
			currentState = UART_IDLE; // ���ص�����״̬
			break;

		default:					  // ȷ��״̬�Ϸ�
			currentState = UART_IDLE; // Ĭ�Ϸ��ص�����״̬
			break;
		}
		*/
	}
}

//==================================================
//					void IO_Init();
//==================================================
void IO_Init()
{
	P_SW2 |= 0x80; // ��չ�Ĵ���(XFR)����ʹ��
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

void delay50us() //@22.1184MHz
{
	unsigned int i;

	for (i = 0; i < 1106; i++)
	{
		_nop_();
	}
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
// ����: ȫ��LED��ʼ�����ӳ�5΢��.
// ����: none.
// ����: none.
//========================================================================
void rgb_init()
{
	P5M0 = 0x00;
	P5M1 = 0x00;
	delay50us();
}

//========================================================================
// ����: 1�룬�ߵ�ƽ850ns �͵�ƽ400ns �������150ns.
// ��22.1184MHz��19��_nop_Ϊ858.99ns
//========================================================================
void rgb_set_up()
{
	RGB_PIN = 1;
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
	_nop_();
	_nop_();
	RGB_PIN = 0;
}

//========================================================================
// ����: 0�룬�ߵ�ƽ400ns �͵�ƽ850ns �������150ns.
// ��22.1184MHz��9��_nop_Ϊ406.89ns
//========================================================================
void rgb_set_down()
{
	RGB_PIN = 1;
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
// ����: ����24λ����.
// ����: ��ɫ8λ����ɫ8λ����ɫ8λ.
// ����: none.
//========================================================================
void rgb_write(uint8 G8, uint8 R8, uint8 B8)
{
	unsigned int n = 0;
	// ����G8λ
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
	// ����R8λ
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
	// ����B8λ
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
// ����: ��λ.
// ����: none.
// ����: none.
//========================================================================
void rgb_rst()
{
	RGB_PIN = 0;
	delay50us();
}

// //========================================================================
// // ����: ��24λ����GRB��תRGB.
// // ����: none.
// // ����: none.
// //========================================================================
// void Set_Colour(uint8 r, uint8 g, uint8 b)
// {
// 	uint8 i;
// 	for (i = 0; i < RGB_NUMLEDS; i++)
// 	{
// 		_rgb_buf_R[i] = r; //����
// 		_rgb_buf_G[i] = g;
// 		_rgb_buf_B[i] = b;
// 	}
// 	for (i = 0; i < RGB_NUMLEDS; i++)
// 	{
// 		rgb_write(_rgb_buf_G[i], _rgb_buf_R[i], _rgb_buf_B[i]);//������ʾ
// 	}
// }

//========================================================================
// ����: ����ָ�������ʾ��ɫ.
// ����: �ڼ���RGB�ƣ�Rֵ,Gֵ,Bֵ.
// ����: none.
//========================================================================
void rgb_set_color(uint8 num, uint8 r, uint8 g, uint8 b)
{
	uint8 i;
	for (i = 0; i < RGB_NUMLEDS; i++)
	{
		_rgb_buf_R[num] = r; // ����
		_rgb_buf_G[num] = g;
		_rgb_buf_B[num] = b;
	}
	for (i = 0; i < RGB_NUMLEDS; i++)
	{
		rgb_write(_rgb_buf_G[i], _rgb_buf_R[i], _rgb_buf_B[i]); // ������ʾ
	}
}

// //========================================================================
// // ����: ��ɫ����24λ����ַ�.
// // ����: none.
// // ����: none.
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
// ����: �ӳ�t΢�룬���ɴ�t = 1000us.
// ����: t - �ӳ�ʱ�䣨΢�룩.
// ����: none.
//========================================================================
void rgb_hal_delay(unsigned int t)
{
	unsigned int i, j;

	// ����1΢����Ҫ��nop����
	// Լ22��nop������1΢��
	unsigned int nop_count = 22;

	for (i = 0; i < t; i++)
	{
		for (j = 0; j < nop_count; j++)
		{
			_nop_(); // ÿ��ѭ����Լ45.21����
		}
	}
}

//========================================================================
// ����: �ӳ�5΢��.
// ����: none.
// ����: none.
//========================================================================
void rgb_show(uint8 num, uint8 r, uint8 g, uint8 b)
{
	rgb_rst();
	rgb_hal_delay(30);
	rgb_set_color(num, r, g, b);
}

//=================================================
//					��ɫLED
//==================================================
// �ر���ɫLED
void Shutdown_LED()
{
	P00 = 1;
	P01 = 1;
	P02 = 1;
	rgb_show(1, 0, 0, 0);
	rgb_show(2, 0, 0, 0);
}

// ���ڿ�����ɫLED�Լ�WS2812
void Ctrl_LED(unsigned char *rxdata)
{

	Shutdown_LED(); // �ر�ȫ��LED

	// ��ɫLED
	// ��ִ��ָ�LEDA��SHUT��LEDB��LEDR��LEDG
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
	/*
	else if (strcmp(rxdata, "RGBR") == 0)//ִ�в���
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
	*/

	// WS2812
	// ��ִ��ָ�RALL��RAO1��RAO2��RAC1��RAC2��RGB1��RGB2��RGBR��RGBG��RGBB��RGBr��RGBg��RGBb
	else if (strcmp(rxdata, "RALL") == 0)
	{
		rgb_show(0, 255, 255, 255);
		rgb_show(1, 255, 255, 255);
	}
	else if (strcmp(rxdata, "RAO1") == 0)
	{
		rgb_show(0, 255, 255, 255);
	}
	else if (strcmp(rxdata, "RAO2") == 0)
	{
		rgb_show(1, 255, 255, 255);
	}
	else if (strcmp(rxdata, "RAC1") == 0)
	{
		rgb_show(0, 0, 0, 0);
	}
	else if (strcmp(rxdata, "RAC2") == 0)
	{
		rgb_show(1, 0, 0, 0);
	}
	else if (strcmp(rxdata, "RGB1") == 0)
	{
		rgb_show(0, 255, 255, 255);
	}
	else if (strcmp(rxdata, "RGB2") == 0)
	{
		rgb_show(1, 255, 255, 255);
	}
	else if (strcmp(rxdata, "RGBR") == 0)
	{
		rgb_show(0, 255, 0, 0);
	}
	else if (strcmp(rxdata, "RGBG") == 0)
	{
		rgb_show(0, 0, 255, 0);
	}
	else if (strcmp(rxdata, "RGBB") == 0)
	{
		rgb_show(0, 0, 0, 255);
	}
	else if (strcmp(rxdata, "RGBr") == 0)
	{
		rgb_show(1, 255, 0, 0);
	}
	else if (strcmp(rxdata, "RGBg") == 0)
	{
		rgb_show(1, 0, 255, 0);
	}
	else if (strcmp(rxdata, "RGBb") == 0)
	{
		rgb_show(1, 0, 0, 255);
	}
}

//========================================================================
// ����: void PrintString1(u8 *puts)
// ����: ����1�����ַ���������
// ����: puts:  �ַ���ָ��.
// ����: none.
// �汾: VER1.0
// ����: 2014-11-28
// ��ע:
//========================================================================
void PrintString1(u8 *puts) // ����һ���ַ���
{
	for (; *puts != 0; puts++) // ����ֹͣ��0����
	{
		SBUF = *puts;
		B_TX1_Busy = 1;
		while (B_TX1_Busy)
			;
	}
}

//========================================================================
// ����: SetTimer2Baudraye(u16 dat)
// ����: ����Timer2�������ʷ�������
// ����: dat: Timer2����װֵ.
// ����: none.
// �汾: VER1.0
// ����: 2014-11-28
// ��ע:
//========================================================================
void SetTimer2Baudraye(u16 dat) // ѡ������, 2: ʹ��Timer2��������, ����ֵ: ʹ��Timer1��������.
{
	AUXR &= ~(1 << 4); // Timer stop
	AUXR &= ~(1 << 3); // Timer2 set As Timer
	AUXR |= (1 << 2);  // Timer2 set as 1T mode
	T2H = dat / 256;
	T2L = dat % 256;
	IE2 &= ~(1 << 2); // ��ֹ�ж�
	AUXR |= (1 << 4); // Timer run enable
}

//========================================================================
// ����: void UART1_config(u8 brt)
// ����: UART1��ʼ��������
// ����: brt: ѡ������, 2: ʹ��Timer2��������, ����ֵ: ʹ��Timer1��������.
// ����: none.
// �汾: VER1.0
// ����: 2014-11-28
// ��ע:
//========================================================================
void UART1_config(u8 brt) // ѡ������, 2: ʹ��Timer2��������, ����ֵ: ʹ��Timer1��������.
{
	/*********** ������ʹ�ö�ʱ��2 *****************/
	if (brt == 2)
	{
		AUXR |= 0x01; // S1 BRT Use Timer2;
		SetTimer2Baudraye(65536UL - (MAIN_Fosc / 4) / Baudrate1);
	}

	/*********** ������ʹ�ö�ʱ��1 *****************/
	else
	{
		TR1 = 0;
		AUXR &= ~0x01;	   // S1 BRT Use Timer1;
		AUXR |= (1 << 6);  // Timer1 set as 1T mode
		TMOD &= ~(1 << 6); // Timer1 set As Timer
		TMOD &= ~0x30;	   // Timer1_16bitAutoReload;
		TH1 = (u8)((65536UL - (MAIN_Fosc / 4) / Baudrate1) / 256);
		TL1 = (u8)((65536UL - (MAIN_Fosc / 4) / Baudrate1) % 256);
		ET1 = 0;		  // ��ֹ�ж�
		INTCLKO &= ~0x02; // �����ʱ��
		TR1 = 1;
	}
	/*************************************************/

	SCON = (SCON & 0x3f) | 0x40; // UART1ģʽ, 0x00: ͬ����λ���, 0x40: 8λ����,�ɱ䲨����, 0x80: 9λ����,�̶�������, 0xc0: 9λ����,�ɱ䲨����
	//  PS  = 1;    //�����ȼ��ж�
	ES = 1;	 // �����ж�
	REN = 1; // �������
	P_SW1 &= 0x3f;
	P_SW1 |= 0x00; // UART1 switch to, 0x00: P3.0 P3.1, 0x40: P3.6 P3.7, 0x80: P1.6 P1.7, 0xC0: P4.3 P4.4
	//  PCON2 |=  (1<<4);   //�ڲ���·RXD��TXD, ���м�, ENABLE,DISABLE

	B_TX1_Busy = 0;
	TX1_Cnt = 0;
	RX1_Cnt = 0;
}

//========================================================================
// ����: void UART1_int (void) interrupt UART1_VECTOR
// ����: UART1�жϺ�����
// ����: nine.
// ����: none.
// �汾: VER1.0
// ����: 2014-11-28
// ��ע:
//========================================================================
void UART1_int(void) interrupt 4
{
	if (RI)
	{
		RI = 0;
		RX1_Buffer[RX1_Cnt] = SBUF;
		if (RX1_Buffer[RX1_Cnt] == '#')
		{
			receiving_complete = 1; // ????????
		}
		else
		{
			if (++RX1_Cnt >= UART1_BUF_LENGTH)
				RX1_Cnt = 0; // ????
		}
	}

	if (TI)
	{
		TI = 0;
		B_TX1_Busy = 0;
	}
}

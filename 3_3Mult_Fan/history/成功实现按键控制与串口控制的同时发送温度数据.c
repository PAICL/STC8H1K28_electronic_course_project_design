//===============================================================
// 本项目基于STC8H1K28开发板以及STC官方Damo程序开发
// 本程序功能有三种控制，分别为按键控制，串口控制以及温度控制风扇转速
// 按键控制主要通过按键切换风扇转速
// 串口控制主要通过串口输出"FAN0#"到"FAN9#"来控制P10端口上的PWM控制占空比(1kHz)?
// 温度控制主要通过检测当前温度是否达到判定某一个值来决定风扇转速的值
// 频率为22.1184MHz，波特率为115200
// DATA:2024/10/23
//===============================================================
#include "stc8h.h"
#include "intrins.h"
//#include "string.h"
#define MAIN_Fosc 22118400L

typedef unsigned char u8;
typedef unsigned int u16;
typedef unsigned long u32;
typedef signed char int8;
typedef signed int int16;
typedef signed long int32;
typedef unsigned char uint8;
typedef unsigned int uint16;
typedef unsigned long uint32;
#define TRUE 1
#define FALSE 0
// #define NULL 0

#define Baudrate1 115200L
#define UART1_BUF_LENGTH 8
bit receiving_complete = 0;
u8 TX1_Cnt;
u8 RX1_Cnt;
bit B_TX1_Busy;
u8 xdata RX1_Buffer[UART1_BUF_LENGTH];
void UART1_config(u8 brt);
void PrintString1(u8 *puts);

void IO_Init();

//@22.1184MHz
void delay_us(uint16 us)
{
    while (us--)
    {
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
        _nop_();
        _nop_();
        _nop_();
    }
}
void delay_ms(uint16 ms)
{
    while (ms--)
    {
        unsigned char i;
        for (i = 0; i < 18; i++)
        {
            delay_us(56);
        }
    }
}
void Delaykey1();
void Delaykey2();

void Shutdown_LED();
void Ctrl_LED(unsigned char *rxdata);

void PrintString1(u8 *puts);
void SetTimer2Baudraye(u16 dat);
void UART1_config(u8 brt);

enum UART_State
{
    UART_IDLE,
    UART_RECEIVE,
    UART_SEND
};

#define Timer0_Reload (65536UL - (MAIN_Fosc / 1000))

sbit key1 = P3 ^ 2;
sbit key2 = P3 ^ 3;
volatile unsigned char Key_Switch = 0; // P32控制风扇工作模式，默认为按键控制[Key_Switch = 0]/串口控制[Key_Switch = 1]/温度控制[Key_Switch = 2]
volatile unsigned char Key_State = 0;
volatile unsigned char Uart_State = 0;
volatile unsigned char Temp_State = 0;
volatile unsigned char Fan_State = 0;
#define PWM1_1 0x00
#define PWM1_2 0x01
#define PWM1_3 0x02

#define ENO1P 0x01
#define ENO1N 0x02

#define PWM_PERIOD 1023

bit B_1ms;

u16 PWM1_Duty;

bit PWM1_Flag;

void UpdatePwm(void);

//===============================================
//==========DS18B20===========
unsigned char TMPL, TMPH;
sbit DQ = P2 ^ 3;
// DS18B20复位
unsigned DS18B20_Reset()
{
    unsigned char x;
    DQ = 1;
    DQ = 0;
    delay_us(700);
    DQ = 1;
    delay_us(30);
    x = DQ;
    while (!DQ)
        ;
    return x;
}
// DS18B20读一位数据函数
unsigned DS18B20_readbit()
{
    bit x;
    unsigned i;
    DQ = 1;
    i++;
    DQ = 0;
    delay_us(6);
    DQ = 1;
    i++;
    i++;
    x = DQ;
    delay_us(50);
    return (x);
}
// DS18B20写一位函数
void DS18B20_writebit(unsigned aa)
{
    unsigned char i;
    DQ = 1;
    i++;
    DQ = 0;
    i++;
    DQ = aa;
    delay_us(50);
    DQ = 1;
}
// DS18B20读一个字节函数
unsigned DS18B20_readbyte()
{
    unsigned i, j, dat;
    dat = 0;
    j = 1;
    for (i = 0; i < 8; i++)
    {
        if (DS18B20_readbit())
        {
            dat = dat + (j << i);
        }
    }
    return (dat);
}
// DS18B20写一个字节函数
void DS18B20_writebyte(unsigned char dat)
{
    unsigned int temp;
    unsigned char j;
    for (j = 0; j < 8; j++)
    {
        temp = dat >> j;
        temp = temp & 0x01;
        DS18B20_writebit(temp);
    }
}
// DS18b20读供电方式
bit DS18B20_readpower()
{
    bit x;
    while (DS18B20_Reset())
        ;
    DS18B20_writebyte(0xcc);
    DS18B20_writebyte(0xb4);
    x = DQ;
    delay_us(10);
    return x;
}
// DS18B20精度设置函数
void DS18B20_SetResolution(unsigned char res)
{
    switch (res)
    {
    case 9:
        res = 0;
        break;
    case 10:
        res = 1;
        break;
    case 11:
        res = 2;
        break;
    case 12:
        res = 3;
        break;
    }
    while (DS18B20_Reset())
        ;
    DS18B20_writebyte(0x4e);
    DS18B20_writebyte(0xff);
    DS18B20_writebyte(0xff);
    DS18B20_writebyte(0x1f | (res << 5));
}
// DS18B20温度读取函数
long DS18B20_GetTemperature()
{
    long Temp;
    while (DS18B20_Reset())
        ;
    DS18B20_writebyte(0xcc);
    DS18B20_writebyte(0x44);
    while (DS18B20_Reset())
        ;
    DS18B20_writebyte(0xcc);
    DS18B20_writebyte(0xbe);
    TMPL = DS18B20_readbyte();
    TMPH = DS18B20_readbyte();
    Temp = TMPH << 8 | TMPL;
    Temp = Temp * 0.0625;
    return Temp;
}


int my_strcmp(const char *str1, const char *str2)
{
    while (*str1 != '\0' && *str2 != '\0')
    {
        if (*str1 != *str2)
        {
            return (unsigned char)*str1 - (unsigned char)*str2;
        }
        str1++;
        str2++;
    }
    return (unsigned char)*str1 - (unsigned char)*str2;
}


void longToString(long value, char *buffer) {
    int i ,j= 0;
    int isNegative = 0;

    // 处理负数
    if (value < 0) {
        isNegative = 1;
        value = -value;
    }

    // 将数字转换为字符串
    do {
        buffer[i++] = (value % 10) + '0'; // 获取最低位数字并转换为字符
        value /= 10; // 去掉最低位
    } while (value > 0);

    if (isNegative) {
        buffer[i++] = '-'; // 添加负号
    }

    // 反转字符串
    buffer[i] = '\0'; // 结束字符串
    for (j = 0; j < i / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }
}
void sendTemperature(long temperature) {
    char buffer[20]; // 用于存储转换后的字符串
    longToString(temperature, buffer); // 转换为字符串
    PrintString1(buffer); // 通过打印函数发送
}



void main()
{
    long temperature;//=================

    char RX_Data[UART1_BUF_LENGTH] = 0x00;
    unsigned char i = 0x00;
    enum UART_State currentState = UART_IDLE;

    DS18B20_readpower();//=====================
    DS18B20_SetResolution(12);//======================

    IO_Init();

    UART1_config(1);
    EA = 1;
    PWM1_Flag = 0;
    PWM1_Duty = 0;

    AUXR = 0x80;
    TH0 = (u8)(Timer0_Reload / 256);
    TL0 = (u8)(Timer0_Reload % 256);
    ET0 = 1;
    TR0 = 1;

    IT0 = 1;
    EX0 = 1;
    IT1 = 1;
    EX1 = 1;
    EA = 1;

    PWMA_CCER1 = 0x00;
    PWMA_CCER2 = 0x00;
    PWMA_CCMR1 = 0x68;
    PWMA_CCMR2 = 0x68;
    PWMA_CCMR3 = 0x68;
    PWMA_CCMR4 = 0x68;
    PWMA_CCER1 = 0x55;
    PWMA_CCER2 = 0x55;

    PWMA_ARRH = (u8)(PWM_PERIOD >> 8);
    PWMA_ARRL = (u8)PWM_PERIOD;

    PWMA_ENO = 0x00;
    PWMA_ENO |= ENO1P;
    PWMA_ENO |= ENO1N;

    PWMA_PS = 0x00;
    PWMA_PS |= P10;

    PWMA_BKR = 0x80;
    PWMA_CR1 |= 0x01;

    P40 = 0;
    UART1_config(1);

    delay_ms(1);

    EA = 1;

    while (1)
    {
        if (Key_Switch == 0)
        {
            if (key2 == 0)
            {
                Delaykey2();
                if (key2 == 0)
                {
                    Key_State++;
                    if (Key_State > 4)
                    {
                        Key_State = 0;
                    }
                }
            }
        }
        else if (Key_Switch == 1)
        {
            char RX_Data[UART1_BUF_LENGTH];
            unsigned char i;

            if (receiving_complete)
            {
                for (i = 0; i < RX1_Cnt; i++)
                {
                    RX_Data[i] = RX1_Buffer[i];
                }
                RX_Data[RX1_Cnt] = '\0';

                Ctrl_LED(RX_Data);

                RX1_Cnt = 0;
                receiving_complete = 0;
            }
        }
        else if(Key_Switch == 2)
        {
            temperature = DS18B20_GetTemperature();
            sendTemperature(temperature);
            delay_ms(200);
        }
    }
}

void IO_Init()
{
    P_SW2 |= 0x80;
    P0M1 = 0x00;
    P0M0 = 0x00;
    P1M1 = 0x00;
    P1M0 = 0x00;
    P2M1 = 0x00;
    P2M0 = 0x00;
    P3M1 = 0x00;
    P3M0 = 0x00;
}

void Delaykey1()
{
    u8 ms = 50;
    while (ms--)
    {
        unsigned char i;
        for (i = 0; i < 18; i++)
        {
            delay_us(56);
        }
    }
}

void Delaykey2()
{
    u8 ms = 60;
    while (ms--)
    {
        unsigned char i;
        for (i = 0; i < 18; i++)
        {
            delay_us(56);
        }
    }
}

void Shutdown_LED()
{
    P00 = 1;
    P01 = 1;
    P02 = 1;
}

void Ctrl_LED(unsigned char *rxdata)
{

    Shutdown_LED();
    //=============================
    if (my_strcmp(rxdata, "FAN0") == 0)
    {
        Uart_State = 0;
    }
    else if (my_strcmp(rxdata, "FAN1") == 0)
    {
        Uart_State = 1;
    }
    else if (my_strcmp(rxdata, "FAN2") == 0)
    {
        Uart_State = 2;
    }
    else if (my_strcmp(rxdata, "FAN3") == 0)
    {
        Uart_State = 3;
    }
    else if (my_strcmp(rxdata, "FAN4") == 0)
    {
        Uart_State = 4;
    }
    else if (my_strcmp(rxdata, "FAN5") == 0)
    {
        Uart_State = 5;
    }
    else if (my_strcmp(rxdata, "FAN6") == 0)
    {
        Uart_State = 6;
    }
    else if (my_strcmp(rxdata, "FAN7") == 0)
    {
        Uart_State = 7;
    }
    else if (my_strcmp(rxdata, "FAN8") == 0)
    {
        Uart_State = 8;
    }
    else if (my_strcmp(rxdata, "FAN9") == 0)
    {
        Uart_State = 9;
    }
}

void PrintString1(u8 *puts)
{
    for (; *puts != 0; puts++)
    {
        SBUF = *puts;
        B_TX1_Busy = 1;
        while (B_TX1_Busy)
            ;
    }
}

void SetTimer2Baudraye(u16 dat)
{
    AUXR &= ~(1 << 4);
    AUXR &= ~(1 << 3);
    AUXR |= (1 << 2);
    T2H = dat / 256;
    T2L = dat % 256;
    IE2 &= ~(1 << 2);
    AUXR |= (1 << 4);
}

void UART1_config(u8 brt)
{
    if (brt == 2)
    {
        AUXR |= 0x01;
        SetTimer2Baudraye(65536UL - (MAIN_Fosc / 4) / Baudrate1);
    }

    else
    {
        TR1 = 0;
        AUXR &= ~0x01;
        AUXR |= (1 << 6);
        TMOD &= ~(1 << 6);
        TMOD &= ~0x30;
        TH1 = (u8)((65536UL - (MAIN_Fosc / 4) / Baudrate1) / 256);
        TL1 = (u8)((65536UL - (MAIN_Fosc / 4) / Baudrate1) % 256);
        ET1 = 0;
        INTCLKO &= ~0x02;
        TR1 = 1;
    }

    SCON = (SCON & 0x3f) | 0x40;
    ES = 1;
    REN = 1;

    P_SW1 &= 0x3f;
    P_SW1 |= 0x00;

    B_TX1_Busy = 0;
    TX1_Cnt = 0;
    RX1_Cnt = 0;
}

void UART1_int(void) interrupt 4
{
    if (RI)
    {
        RI = 0;
        RX1_Buffer[RX1_Cnt] = SBUF;
        if (RX1_Buffer[RX1_Cnt] == '#')
        {
            receiving_complete = 1;
        }
        else
        {
            if (++RX1_Cnt >= UART1_BUF_LENGTH)
            {
                RX1_Cnt = 0; // ?陇?0?8?0鈥?0?0?1?0?6?0?4?0鈥?6?0?2
            }
        }
    }
    if (TI)
    {
        TI = 0;
        B_TX1_Busy = 0;
    }
}

void timer0(void) interrupt 3 using 2
{
    if (Key_Switch == 0)
    {
        P00 = 0;
        P01 = P02 = 1;
        Uart_State = 0;
        Temp_State = 0;
        if (Key_State == 0)
        {
            PWM1_Duty = 0;
        }
        if (Key_State == 1)
        {
            PWM1_Duty = 341;
        }
        if (Key_State == 2)
        {
            PWM1_Duty = 682;
        }
        if (Key_State == 3)
        {
            PWM1_Duty = 1023;
        }
    }

    if (Key_Switch == 1)
    {
        P01 = 0;
        P00 = P02 = 1;
        Key_State = 0;
        Temp_State = 0;
        if (Uart_State == 0)
        {
            PWM1_Duty = 0;
        }
        else if (Uart_State == 1)
        {
            PWM1_Duty = 113;
        }
        else if (Uart_State == 2)
        {
            PWM1_Duty = 227;
        }
        else if (Uart_State == 3)
        {
            PWM1_Duty = 341;
        }
        else if (Uart_State == 4)
        {
            PWM1_Duty = 454;
        }
        else if (Uart_State == 5)
        {
            PWM1_Duty = 568;
        }
        else if (Uart_State == 6)
        {
            PWM1_Duty = 682;
        }
        else if (Uart_State == 7)
        {
            PWM1_Duty = 795;
        }
        else if (Uart_State == 8)
        {
            PWM1_Duty = 909;
        }
        else if (Uart_State == 9)
        {
            PWM1_Duty = 1023;
        }
    }
    if (Key_Switch == 2)
    {
        P02 = 0;
        P00 = P01 = 1;
        Key_State = 0;
        Uart_State = 0;
        PWM1_Duty = 1023;
    }
    UpdatePwm();
}

void UpdatePwm(void)
{
    PWMA_CCR1H = (u8)(PWM1_Duty >> 8);
    PWMA_CCR1L = (u8)PWM1_Duty;
}

void Int0_isr() interrupt 0 using 0
{
    if (key1 == 0)
    {
        Delaykey1();
        if (key1 == 0)
        {
            Key_Switch++;
            if (Key_Switch > 2)
            {
                Key_Switch = 0;
            }
        }
    }
}
/*
void Int1_isr() interrupt 2 using 1
{
    if (key2 == 0)
    {
        Delaykey2();
        if (key2 == 0)
        {
            Key_State++;
            if (Key_State > 4)
            {
                Key_State = 0;
            }
        }
    }
}
*/

/*=============================================================
本项目基于STC8H1K28

该项目需要 [显示模块TM1650] 以及 [温度传感器DS18B20]
[显示模块TM1650] 为 [IIC] 传输协议
引脚接口为 [SDA],[SCL],[GND],[VDD]
连接说明如下
[TM1650]    [开发板]
  [SDA]----->[P14]
  [SCL]----->[P15]
  [GND]----->[GND]
  [VDD]----->[3.3v]

[温度传感器DS18B20]引脚接口为[VDD],[OUT],[GND]
(有些学校温度传感器开发板对应为[+], [out],[-],请自行转换)
连接说明如下
[DS18B20]   [开发板]
  [VDD]----->[3.3]
  [OUT]----->[P23]
  [GND]----->[GND]

烧录时需要注意将频率设置为[6.00MHz],否则无法运行

Data:2024/10/19
*///===========================================================

#include "stc8h.h"
#include "intrins.h"
//6MHz

//==========IO_Init============
sbit key1 = P3 ^ 2;
sbit key2 = P3 ^ 3;

void IO_Init()
{
    P0M0 = 0x00;
    P0M1 = 0x00;
    P2M0 = 0x00;
    P2M1 = 0x00;
    P3M0 = 0x00;
    P3M1 = 0x00;
}
//=========IO_Init============

//=============delay==========
// 微秒级延迟函数
void delay_us(unsigned int us)
{
    while (us--)
    {
        _nop_();
    }
}

// 毫秒级延迟函数
void delay_ms(unsigned int ms)
{
    while (ms--)
    {
        delay_us(1000);
    }
}

//==========delay============

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

//==========DS18B20====================

//====================TM1650=======================
unsigned int tab[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66,        // 0,1,2,3,4
                      0x6D, 0x7D, 0x07, 0x7F, 0x6F,        // 5,6,7,8,9
                      0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71}; // A,B,C,D,E,F

bit busy; // 全局变量

/***********初始化***********/
void I2C_init()
{
    P_SW2 = 0x80; // 允许访问扩展的SFR
    P1M1 = 0;
    P1M0 = 0;
    I2CCFG = 0xe0;  // 允许I2C，主机模式，等待时钟数16
    I2CMSST = 0x00; // 清空状态标志位
    EA = 1;         // 开总中断
}

/***********开始信号***********/
void Start()
{
    busy = 1;
    I2CMSCR = 0x81; // 发送START命令
    while (busy)
        ;
}

/***********发送一个字节数据***********/
void SendData(unsigned char dat)
{
    I2CTXD = dat; // 写数据到数据缓冲区
    busy = 1;
    I2CMSCR = 0x82; // 发送SEND命令
    while (busy)
        ;
}

/***********等待接收从机返回接收成功的信号***********/
void RecvACK()
{
    busy = 1;
    I2CMSCR = 0x83; // 发送读ACK命令
    while (busy)
        ;
}

/***********停止信号***********/
void Stop()
{
    busy = 1;
    I2CMSCR = 0x86; // 发送STOP命令
    while (busy)
        ;
}

/***********中断，清除标志位***********/
void I2C_Isr() interrupt 24
{
    if (I2CMSST & 0x40)
    {
        I2CMSST &= ~0x40; // 清中断标志
        busy = 0;
    }
}

//=========================
// 描述：TM1650写函数
// 参数：add：地址；dat：数据
// 返回：none
//==========================
void TM1650_write_data(unsigned int add, unsigned int dat)
{
    Start();
    SendData(add); // 添加发送地址
    RecvACK();
    SendData(dat);
    RecvACK();
    Stop();
}

//=========================
// 描述：TM1650初始化
// 参数：none
// 返回：none
//========================
void TM1650_Init()
{
    I2C_init();
    TM1650_write_data(0x48, 0x31); // 开显示、8段显示方式、3级亮度
}

//=========================
// 描述：TM1650清除
// 参数：none
// 返回：none
//========================
void TM1650_clear()
{
    unsigned char i;
    for (i = 0; i < 4; i++)
    {
        TM1650_write_data(0x68 + i * 2, 0);
    }
}
//===========TM1650=====================

void main()
{
    long temperature;
    IO_Init();
    TM1650_Init();
    TM1650_clear();
    DS18B20_readpower();
    DS18B20_SetResolution(12);
    delay_ms(100);
    delay_us(1);

    while (1)
    {
        temperature = DS18B20_GetTemperature();
        TM1650_write_data(0x68, tab[(temperature / 1000) % 10]); // 显示千位
        TM1650_write_data(0x6A, tab[(temperature / 100) % 10]);  // 显示百位
        TM1650_write_data(0x6C, tab[(temperature / 10) % 10]);   // 显示十位
        TM1650_write_data(0x6E, tab[temperature % 10]);

        P00 = 0; 
        delay_ms(100);
        P00 = 1;
        delay_ms(100);

        P01 = 0;
        delay_ms(100);
        P01 = 1;
        delay_ms(100);

        P02 = 0;
        delay_ms(100);
        P02 = 1;
        delay_ms(100);
    }
}

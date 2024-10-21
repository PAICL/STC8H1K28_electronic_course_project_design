/*=============================================================
����Ŀ����STC8H1K28

����Ŀ��Ҫ [��ʾģ��TM1650] �Լ� [�¶ȴ�����DS18B20]
[��ʾģ��TM1650] Ϊ [IIC] ����Э��
���Žӿ�Ϊ [SDA],[SCL],[GND],[VDD]
����˵������
[TM1650]    [������]
  [SDA]----->[P14]
  [SCL]----->[P15]
  [GND]----->[GND]
  [VDD]----->[3.3v]

[�¶ȴ�����DS18B20]���Žӿ�Ϊ[VDD],[OUT],[GND]
(��ЩѧУ�¶ȴ������������ӦΪ[+], [out],[-],������ת��)
����˵������
[DS18B20]   [������]
  [VDD]----->[3.3]
  [OUT]----->[P23]
  [GND]----->[GND]

��¼ʱ��Ҫע�⽫Ƶ������Ϊ[6.00MHz],�����޷�����

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
// ΢�뼶�ӳٺ���
void delay_us(unsigned int us)
{
    while (us--)
    {
        _nop_();
    }
}

// ���뼶�ӳٺ���
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

// DS18B20��λ
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

// DS18B20��һλ���ݺ���
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

// DS18B20дһλ����
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

// DS18B20��һ���ֽں���
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

// DS18B20дһ���ֽں���
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

// DS18b20�����緽ʽ
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

// DS18B20�������ú���
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

// DS18B20�¶ȶ�ȡ����
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

bit busy; // ȫ�ֱ���

/***********��ʼ��***********/
void I2C_init()
{
    P_SW2 = 0x80; // ���������չ��SFR
    P1M1 = 0;
    P1M0 = 0;
    I2CCFG = 0xe0;  // ����I2C������ģʽ���ȴ�ʱ����16
    I2CMSST = 0x00; // ���״̬��־λ
    EA = 1;         // �����ж�
}

/***********��ʼ�ź�***********/
void Start()
{
    busy = 1;
    I2CMSCR = 0x81; // ����START����
    while (busy)
        ;
}

/***********����һ���ֽ�����***********/
void SendData(unsigned char dat)
{
    I2CTXD = dat; // д���ݵ����ݻ�����
    busy = 1;
    I2CMSCR = 0x82; // ����SEND����
    while (busy)
        ;
}

/***********�ȴ����մӻ����ؽ��ճɹ����ź�***********/
void RecvACK()
{
    busy = 1;
    I2CMSCR = 0x83; // ���Ͷ�ACK����
    while (busy)
        ;
}

/***********ֹͣ�ź�***********/
void Stop()
{
    busy = 1;
    I2CMSCR = 0x86; // ����STOP����
    while (busy)
        ;
}

/***********�жϣ������־λ***********/
void I2C_Isr() interrupt 24
{
    if (I2CMSST & 0x40)
    {
        I2CMSST &= ~0x40; // ���жϱ�־
        busy = 0;
    }
}

//=========================
// ������TM1650д����
// ������add����ַ��dat������
// ���أ�none
//==========================
void TM1650_write_data(unsigned int add, unsigned int dat)
{
    Start();
    SendData(add); // ��ӷ��͵�ַ
    RecvACK();
    SendData(dat);
    RecvACK();
    Stop();
}

//=========================
// ������TM1650��ʼ��
// ������none
// ���أ�none
//========================
void TM1650_Init()
{
    I2C_init();
    TM1650_write_data(0x48, 0x31); // ����ʾ��8����ʾ��ʽ��3������
}

//=========================
// ������TM1650���
// ������none
// ���أ�none
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
        TM1650_write_data(0x68, tab[(temperature / 1000) % 10]); // ��ʾǧλ
        TM1650_write_data(0x6A, tab[(temperature / 100) % 10]);  // ��ʾ��λ
        TM1650_write_data(0x6C, tab[(temperature / 10) % 10]);   // ��ʾʮλ
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

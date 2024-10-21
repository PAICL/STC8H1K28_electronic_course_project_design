/*========================================================================
����Ŀ����STC8H1K28�Լ��ٷ������ĵ����п���

��Ŀ����ɫLEDΪP00,P01,P02�˿�
��ͨ�����ڷ���4Ϊָ���ڿ���LED��
���ڿɷ���ָ������
LEDA
�������е�
SHUT
�ر����е�
LEDB
����������
LEDR
���������
LEDG
�������̵�
RGBR
��ˮ��
ע���ڴ��ڷ���ʱֻ�ܷ���4Ϊָ�������ͷ�4Ϊָ��ᵼ�½���ָ�������ָ�ͬ�����º���ָ����ղ�ƥ��
���������ں�������õ��޸������������ɴ��ڸ����⣬�����Ҫ�޸�����պ������������滻���޸���Ӧ���ܺ�������

����ʱ, ѡ��ʱ�� 22.1184MHz�������޷�����
Data��2024/10/20
*///======================================================================

#include "stc8h.h"
#include <intrins.h>
#include "string.h"
#define MAIN_Fosc 22118400L // ������ʱ�ӣ���ȷ����115200�����ʣ�

typedef unsigned char u8;
typedef unsigned int u16;
typedef unsigned long u32;

#define Baudrate1 115200L
#define UART1_BUF_LENGTH 4

void Delay500ms()
{
    unsigned char i, j, k;
    _nop_();
    _nop_();
    i = 43;
    j = 6;
    k = 5;
    do
    {
        do
        {
            while (--k)
                ;
        } while (--j);
    } while (--i);
}

u8 TX1_Cnt;     // ���ͼ���
u8 RX1_Cnt;     // ���ռ���
bit B_TX1_Busy; // ����æ��־

u8 xdata RX1_Buffer[UART1_BUF_LENGTH]; // ���ջ���

void UART1_config(u8 brt); // ѡ������, 2: ʹ��Timer2��������, ����ֵ: ʹ��Timer1��������.
void PrintString1(u8 *puts);

void Shutdown_LED()
{
    P00 = 1;
    P01 = 1;
    P02 = 1;
}

void Ctrl_LED(unsigned char *rxdata);

//========================================================================
// ����: void main(void)
// ����: ��������
// ����: none.
// ����: none.
// �汾: VER1.0
// ����: 2014-11-28
// ��ע:
//========================================================================
void main(void)
{
    P_SW2 |= 0x80; // ��չ�Ĵ���(XFR)����ʹ��

    P0M1 = 0x30;
    P0M0 = 0x30; // ����P0.4��P0.5Ϊ©����·(ʵ��������������赽3.3V)
    P1M1 = 0x30;
    P1M0 = 0x30; // ����P1.4��P1.5Ϊ©����·(ʵ��������������赽3.3V)
    P2M1 = 0x3c;
    P2M0 = 0x3c; // ����P2.2~P2.5Ϊ©����·(ʵ��������������赽3.3V)
    P3M1 = 0x50;
    P3M0 = 0x50; // ����P3.4��P3.6Ϊ©����·(ʵ��������������赽3.3V)
    P4M1 = 0x3c;
    P4M0 = 0x3c; // ����P4.2~P4.5Ϊ©����·(ʵ��������������赽3.3V)
    P5M1 = 0x0c;
    P5M0 = 0x0c; // ����P5.2��P5.3Ϊ©����·(ʵ��������������赽3.3V)
    P6M1 = 0xff;
    P6M0 = 0xff; // ����Ϊ©����·(ʵ��������������赽3.3V)
    P7M1 = 0x00;
    P7M0 = 0x00; // ����Ϊ׼˫���

    UART1_config(1); // ѡ������, 2: ʹ��Timer2��������, ����ֵ: ʹ��Timer1��������.
    EA = 1;          // �������ж�

    PrintString1("STC8H1K28 UART1, development by Leyue Chen.\r\n"); // UART1����һ���ַ���

    while (1)
    {
        char RX_Data[UART1_BUF_LENGTH]; // �洢���յ�����
        unsigned char i;

        if ((TX1_Cnt != RX1_Cnt) && (!B_TX1_Busy)) // �յ�����, ���Ϳ���
        {
            // SBUF = RX1_Buffer[TX1_Cnt]; // ���յ�������Զ������
            for (i = 0; i < 4; i++)
            {
                RX_Data[i] = RX1_Buffer[i]; // ��������
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

void Ctrl_LED(unsigned char *rxdata)
{
    Shutdown_LED();

    // ʹ��strcmp�Ƚ��ַ���
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
        AUXR &= ~0x01;     // S1 BRT Use Timer1;
        AUXR |= (1 << 6);  // Timer1 set as 1T mode
        TMOD &= ~(1 << 6); // Timer1 set As Timer
        TMOD &= ~0x30;     // Timer1_16bitAutoReload;
        TH1 = (u8)((65536UL - (MAIN_Fosc / 4) / Baudrate1) / 256);
        TL1 = (u8)((65536UL - (MAIN_Fosc / 4) / Baudrate1) % 256);
        ET1 = 0;          // ��ֹ�ж�
        INTCLKO &= ~0x02; // �����ʱ��
        TR1 = 1;
    }
    /*************************************************/

    SCON = (SCON & 0x3f) | 0x40; // UART1ģʽ, 0x00: ͬ����λ���, 0x40: 8λ����,�ɱ䲨����, 0x80: 9λ����,�̶�������, 0xc0: 9λ����,�ɱ䲨����
    //  PS  = 1;    //�����ȼ��ж�
    ES = 1;  // �����ж�
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
        if (++RX1_Cnt >= UART1_BUF_LENGTH)
            RX1_Cnt = 0; // �����
    }

    if (TI)
    {
        TI = 0;
        B_TX1_Busy = 0;
    }
}

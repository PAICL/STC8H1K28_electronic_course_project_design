/*========================================================================
该项目基于STC8H1K28以及官方程序文档进行开发

项目中三色LED为P00,P01,P02端口
可通过串口发送4为指令在控制LED灯
串口可发送指令如下
LEDA
点亮所有灯
SHUT
关闭所有灯
LEDB
仅点亮蓝灯
LEDR
仅点亮红灯
LEDG
仅点亮绿灯
RGBR
流水灯
注意在串口发送时只能发送4为指令，如果发送非4为指令会导致接收指令与控制指令不同步导致后续指令接收不匹配
该问题已在后续代码得到修复，本代码依旧存在该问题，如果需要修复请对照后续程序自行替换或修改相应功能函数即可

下载时, 选择时钟 22.1184MHz，否则无法运行
Data：2024/10/20
*///======================================================================

#include "stc8h.h"
#include <intrins.h>
#include "string.h"
#define MAIN_Fosc 22118400L // 定义主时钟（精确计算115200波特率）

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

u8 TX1_Cnt;     // 发送计数
u8 RX1_Cnt;     // 接收计数
bit B_TX1_Busy; // 发送忙标志

u8 xdata RX1_Buffer[UART1_BUF_LENGTH]; // 接收缓冲

void UART1_config(u8 brt); // 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer1做波特率.
void PrintString1(u8 *puts);

void Shutdown_LED()
{
    P00 = 1;
    P01 = 1;
    P02 = 1;
}

void Ctrl_LED(unsigned char *rxdata);

//========================================================================
// 函数: void main(void)
// 描述: 主函数。
// 参数: none.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注:
//========================================================================
void main(void)
{
    P_SW2 |= 0x80; // 扩展寄存器(XFR)访问使能

    P0M1 = 0x30;
    P0M0 = 0x30; // 设置P0.4、P0.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P1M1 = 0x30;
    P1M0 = 0x30; // 设置P1.4、P1.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P2M1 = 0x3c;
    P2M0 = 0x3c; // 设置P2.2~P2.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P3M1 = 0x50;
    P3M0 = 0x50; // 设置P3.4、P3.6为漏极开路(实验箱加了上拉电阻到3.3V)
    P4M1 = 0x3c;
    P4M0 = 0x3c; // 设置P4.2~P4.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P5M1 = 0x0c;
    P5M0 = 0x0c; // 设置P5.2、P5.3为漏极开路(实验箱加了上拉电阻到3.3V)
    P6M1 = 0xff;
    P6M0 = 0xff; // 设置为漏极开路(实验箱加了上拉电阻到3.3V)
    P7M1 = 0x00;
    P7M0 = 0x00; // 设置为准双向口

    UART1_config(1); // 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer1做波特率.
    EA = 1;          // 允许总中断

    PrintString1("STC8H1K28 UART1, development by Leyue Chen.\r\n"); // UART1发送一个字符串

    while (1)
    {
        char RX_Data[UART1_BUF_LENGTH]; // 存储接收的数据
        unsigned char i;

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
        AUXR &= ~0x01;     // S1 BRT Use Timer1;
        AUXR |= (1 << 6);  // Timer1 set as 1T mode
        TMOD &= ~(1 << 6); // Timer1 set As Timer
        TMOD &= ~0x30;     // Timer1_16bitAutoReload;
        TH1 = (u8)((65536UL - (MAIN_Fosc / 4) / Baudrate1) / 256);
        TL1 = (u8)((65536UL - (MAIN_Fosc / 4) / Baudrate1) % 256);
        ET1 = 0;          // 禁止中断
        INTCLKO &= ~0x02; // 不输出时钟
        TR1 = 1;
    }
    /*************************************************/

    SCON = (SCON & 0x3f) | 0x40; // UART1模式, 0x00: 同步移位输出, 0x40: 8位数据,可变波特率, 0x80: 9位数据,固定波特率, 0xc0: 9位数据,可变波特率
    //  PS  = 1;    //高优先级中断
    ES = 1;  // 允许中断
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

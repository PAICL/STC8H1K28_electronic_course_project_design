
/*************  前代码功能说明    **************
本例程基于STC8H8K64U为主控芯片的实验箱9进行编写测试，STC8H系列芯片可通用参考。
高级PWM定时器 PWM1P/PWM1N,PWM2P/PWM2N,PWM3P/PWM3N,PWM4P/PWM4N 每个通道都可独立实现PWM输出，或者两两互补对称输出。
8个通道PWM设置对应P6的8个端口。
通过P6口上连接的8个LED灯，利用PWM实现呼吸灯效果。
PWM周期和占空比可以根据需要自行设置，最高可达65535。
下载时, 选择时钟 24MHZ (用户可自行修改频率)。
******************************************/

#include "stc8h.h"
#include "intrins.h"

// 定义主时钟频率
#define MAIN_Fosc 22118400L

// 定义数据类型
typedef unsigned char u8;
typedef unsigned int u16;
typedef unsigned long u32;

#define Timer0_Reload (65536UL - (MAIN_Fosc / 1000)) // 定时器0中断频率, 1000次/秒


// key
sbit key1 = P3 ^ 2;
volatile unsigned char ledstate = 1;

void Delay500ms()
{
    unsigned char i, j, k;
    _nop_();
    _nop_();
    i = 43;
    j = 6;
    k = 50;
    do
    {
        do
        {
            while (--k)
                ;
        } while (--j);
    } while (--i);
}

void Delay10ms()
{
    unsigned char i, j;
    _nop_();
    _nop_();
    i = 27;
    j = 104;
    do
    {
        while (--j)
            ;
    } while (--i);
}

void LED_1()
{
    P02 = 1;
    // Delay500ms();
    P00 = 0;
}
void LED_2()
{
    P00 = 1;
    // Delay500ms();
    P01 = 0;
}

void LED_3()
{
    P01 = 1;
    // Delay500ms();
    P02 = 0;
}

#define PWM1_1 0x00
#define PWM1_2 0x01
#define PWM1_3 0x02

#define PWM2_1 0x00
#define PWM2_2 0x04
#define PWM2_3 0x08

#define PWM3_1 0x00
#define PWM3_2 0x10
#define PWM3_3 0x20

#define PWM4_1 0x00
#define PWM4_2 0x40
#define PWM4_3 0x80
#define PWM4_4 0xC0

#define ENO1P 0x01
#define ENO1N 0x02
#define ENO2P 0x04
#define ENO2N 0x08
#define ENO3P 0x10
#define ENO3N 0x20
#define ENO4P 0x40
#define ENO4N 0x80

#define PWM_PERIOD 1023 // 设置PWM周期值

/*************  本地变量声明    **************/
bit B_1ms; // 1ms标志

u16 PWM1_Duty; // PWM1占空比
u16 PWM2_Duty; // PWM2占空比
u16 PWM3_Duty; // PWM3占空比
u16 PWM4_Duty; // PWM4占空比

bit PWM1_Flag; // PWM1标志位
bit PWM2_Flag; // PWM2标志位
bit PWM3_Flag; // PWM3标志位
bit PWM4_Flag; // PWM4标志位

void UpdatePwm(void); // 更新PWM占空比的函数声明

/******************** 主函数 **************************/
void main(void)
{
    P_SW2 |= 0x80; // 启用扩展寄存器(XFR)访问

    // 设置P0、P1、P2、P3、P4、P5和P6引脚为漏极开路
    P0M1 = 0x30;
    P0M0 = 0x30;
    P1M1 = 0x30;
    P1M0 = 0x30;
    P2M1 = 0x3c;
    P2M0 = 0x3c;
    P3M1 = 0x00;
    P3M0 = 0x00;
    P4M1 = 0x3c;
    P4M0 = 0x3c;
    P5M1 = 0x0c;
    P5M0 = 0x0c;
    P6M1 = 0xff;
    P6M0 = 0xff;
    P7M1 = 0x00;
    P7M0 = 0x00;

    // 初始化PWM标志和占空比
    PWM1_Flag = 0;
    PWM2_Flag = 0;
    PWM3_Flag = 0;
    PWM4_Flag = 0;

    PWM1_Duty = 0;    // 初始占空比为0
    PWM2_Duty = 256;  // 初始占空比
    PWM3_Duty = 512;  // 初始占空比
    PWM4_Duty = 1024; // 初始占空比

    // Timer0初始化
    AUXR = 0x80;                     // Timer0设置为1T，16位自动重载定时器
    TH0 = (u8)(Timer0_Reload / 256); // 设置定时器高字节
    TL0 = (u8)(Timer0_Reload % 256); // 设置定时器低字节
    ET0 = 1;                         // 启用Timer0中断
    TR0 = 1;                         // 启动Timer0

    IT0 = 1;
    EX0 = 1;
    EA = 1;

    // 配置PWM通道
    PWMA_CCER1 = 0x00; // 清除CCxE以关闭通道
    PWMA_CCER2 = 0x00;
    PWMA_CCMR1 = 0x68; // 通道模式配置
    PWMA_CCMR2 = 0x68;
    PWMA_CCMR3 = 0x68;
    PWMA_CCMR4 = 0x68;
    PWMA_CCER1 = 0x55; // 使能通道输出和极性
    PWMA_CCER2 = 0x55;

    PWMA_ARRH = (u8)(PWM_PERIOD >> 8); // 设置PWM周期的高字节
    PWMA_ARRL = (u8)PWM_PERIOD;        // 设置PWM周期的低字节

    // 使能PWM输出
    PWMA_ENO = 0x00;
    PWMA_ENO |= ENO1P;
    PWMA_ENO |= ENO1N;
    PWMA_ENO |= ENO2P;
    PWMA_ENO |= ENO2N;
    PWMA_ENO |= ENO3P;
    PWMA_ENO |= ENO3N;
    PWMA_ENO |= ENO4P;
    PWMA_ENO |= ENO4N;

    PWMA_PS = 0x00; // 清空PWM引脚选择位
    PWMA_PS |= P10; // 设置P10为PWM输出引脚

    PWMA_BKR = 0x80;  // 使能主输出
    PWMA_CR1 |= 0x01; // 启动计时

    P40 = 0; // 给LED供电
    EA = 1;  // 启用全局中断

    while (1)
    {
    }
}

/********************** Timer0 1ms中断函数 ************************/
void timer0(void) interrupt 1
{
    if (ledstate == 1)
    {
        PWM1_Duty = 0;
    }
    if (ledstate == 2)
    {
        PWM1_Duty = 341;
    }
    if (ledstate == 3)
    {
        PWM1_Duty = 683;
    }
    if (ledstate == 4)
    {
        PWM1_Duty = 1023;
    }

    /*
        if (!PWM1_Flag)
    {
        PWM1_Duty++;
        if (PWM1_Duty > PWM_PERIOD)
            PWM1_Flag = 1;
    }
    else
    {
        PWM1_Duty--;
        if (PWM1_Duty <= 0)
            PWM1_Flag = 0;
    }
    */

    UpdatePwm(); // 调用函数更新PWM输出
}

/************************** 更新PWM占空比函数 **************************/
void UpdatePwm(void)
{
    // 设置PWM的占空比
    PWMA_CCR1H = (u8)(PWM1_Duty >> 8); // 设置PWM1的高字节
    PWMA_CCR1L = (u8)PWM1_Duty;        // 设置PWM1的低字节
}

/****************** 按键中断 ************************************/
void Int0_isr() interrupt 0
{
    if (key1 == 0)
    {
        Delay10ms();
		Delay10ms();
        if (key1 == 0)
        {
            ledstate++;
            if (ledstate > 4)
            {
                ledstate = 1;
            }
        }
    }
}

/*//=========================================================
本项目基于STC8H1K28开发板

实现不同速度速度流水灯功能
速度由按键P32控制，共四档速度

按键初始化：使用 initialize() 函数对按键引脚进行配置。
延迟函数：
Delay500ms() 用于产生500毫秒的延迟。
Delay10ms() 用于产生10毫秒的延迟。
LED流转效果：
LED_1()、LED_2()、LED_3()、LED_4() 分别定义了不同的LED流转效果。
每个函数通过控制 P00、P01 和 P02 引脚的状态来实现不同的流转效果。
中断服务程序：
Int0_isr() 中断服务程序检测按键按下的状态。
当按键按下时，会增加 ledstate 的值，并切换到相应的LED流转效果。
主函数：
初始化按键和中断。
根据 ledstate 的值选择相应的LED流转效果函数执行。

Data：2024/10/19
*///=========================================================
#include <stc8h.h>
#include <intrins.h>

sbit key1 = P3 ^ 2;
volatile unsigned char ledstate = 1;

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

void initialize()
{
  P0M0 &= 0x00;
  P0M1 &= 0x00;
  P3M0 = 0x00;
  P3M1 = 0x00;
}

void LED_1()
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
void LED_2()
{
  P00 = 0;
  Delay500ms();
  Delay500ms();
  P00 = 1;
  P01 = 0;
  Delay500ms();
  Delay500ms();
  P01 = 1;
  P02 = 0;
  Delay500ms();
  Delay500ms();
  P02 = 1;
}

void LED_3()
{
  P00 = 0;
  Delay500ms();
  Delay500ms();
  Delay500ms();
  P00 = 1;
  P01 = 0;
  Delay500ms();
  Delay500ms();
  Delay500ms();
  P01 = 1;
  P02 = 0;
  Delay500ms();
  Delay500ms();
  Delay500ms();
  P02 = 1;
}

void LED_4()
{
  P00 = 0;
  Delay500ms();
  Delay500ms();
  Delay500ms();
  Delay500ms();
  P00 = 1;
  P01 = 0;
  Delay500ms();
  Delay500ms();
  Delay500ms();
  Delay500ms();
  P01 = 1;
  P02 = 0;
  Delay500ms();
  Delay500ms();
  Delay500ms();
  Delay500ms();
  P02 = 1;
}

void Int0_isr() interrupt 0
{
  if (key1 == 0)
  {
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

void main()
{
  IT0 = 1;
  EX0 = 1;
  EA = 1;
  Delay500ms();
  initialize();
  while (1)
  {
    if (ledstate == 1)
      LED_1();
    if (ledstate == 2)
      LED_2();
    if (ledstate == 3)
      LED_3();
    if (ledstate == 4)
      LED_4();
  }
}
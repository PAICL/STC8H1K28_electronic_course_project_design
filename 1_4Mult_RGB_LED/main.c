/*///////////////////////////////////////////////
该项目基于STC8H1k28开发板

实现P32控制不同速度流水灯或三色LED其中之一依次亮
实现P33控制切换流水灯还是LED单独亮

Data：2024/10/19
*////////////////////////////////////////////////
#include <stc8h.h>
#include <intrins.h>

sbit key1 = P3 ^ 2;
sbit key2 = P3 ^ 3;
volatile unsigned char ledstate1 = 1;
volatile unsigned char ledstate2 = 1;

void Delay500ms()
{
  unsigned char i, j, k;
  _nop_();
  _nop_();
  i = 13;
  j = 6;
  k = 1;
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

void S_LED_1()
{
  P00 = 0;
  P01 = 1;
  P02 = 1;
}

void S_LED_2()
{
  P00 = 1;
  P01 = 0;
  P02 = 1;
}

void S_LED_3()
{
  P00 = 1;
  P01 = 1;
  P02 = 0;
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

void Int0_isr() interrupt 0 using 1
{
  if (key1 == 0)
  {
    Delay10ms();
    if (key1 == 0)
    {
      ledstate1++;
      if (ledstate1 > 4)
      {
        ledstate1 = 1;
      }
    }
  }
}

void Int1_isr() interrupt 2 using 1
{
  if (key2 == 0)
  {
    Delay10ms();
    if (key2 == 0)
    {
      ledstate2++;
      if (ledstate2 > 2)
      {
        ledstate2 = 1;
      }
    }
  }
}

void main()
{
  IT0 = 1;
  IT1 = 1;
  EX0 = 1;
  EX1 = 1;
  EA = 1;
  Delay500ms();
  initialize();
  while (1)
  {
    if (ledstate2 == 1)
    {
      if (ledstate1 == 1)
        LED_1();
      else if (ledstate1 == 2)
        LED_2();
      else if (ledstate1 == 3)
        LED_3();
      else if (ledstate1 == 4)
        LED_4();
    }
    else if (ledstate2 == 2)
    {
      if (ledstate1 > 3)
        ledstate1 = 1;
      if (ledstate1 == 1)
      {

        S_LED_1();
      }
      else if (ledstate1 == 2)
        S_LED_2();
      else if (ledstate1 == 3)
      {
        S_LED_3();
      }
    }
  }
}

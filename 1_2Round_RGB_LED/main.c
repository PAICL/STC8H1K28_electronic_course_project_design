/*//=========================================================
本项目基于STC8H1K28
控制P00、P01、P02端口上的LED，并更具P32上的按键控制LED依次点亮
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

void initialize()
{
  P0M0 &= 0x00;
  P0M1 &= 0x00;
  P3M0 = 0x00;
  P3M1 = 0x00;
}

void LED_1()
{
  P02 = 1;
  //Delay500ms();
  P00 = 0;
}
void LED_2()
{
  P00 = 1;
  //Delay500ms();
  P01 = 0;
}

void LED_3()
{
  P01 = 1;
  //Delay500ms();
  P02 = 0;
}

void Int0_isr() interrupt 0
{
  if (key1 == 0)
  {
    Delay10ms();
    if (key1 == 0)
    {
      ledstate++;
      if (ledstate > 3)
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
  }
}
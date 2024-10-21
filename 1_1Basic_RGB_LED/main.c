/*//=========================================================
本项目基于STC8H1K28
控制P00、P01、P02端口上的LED依次闪烁
Data：2024/10/19
*///=========================================================

#include <stc8h.h>
#include <intrins.h>

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
      while(--k);
    }  
    while(--j);  
  }  
  while(--i);
}

void initialize()  
{  
  P0M0 &= 0x00;
  P0M1 &= 0x00;  
}

void LED1()  
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

void main()  
{  
  initialize();
  while(1)
  {  
    LED1();
  }  
}

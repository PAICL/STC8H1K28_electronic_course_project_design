/*//=========================================================
����Ŀ����STC8H1K28������

ʵ�ֲ�ͬ�ٶ��ٶ���ˮ�ƹ���
�ٶ��ɰ���P32���ƣ����ĵ��ٶ�

������ʼ����ʹ�� initialize() �����԰������Ž������á�
�ӳٺ�����
Delay500ms() ���ڲ���500������ӳ١�
Delay10ms() ���ڲ���10������ӳ١�
LED��תЧ����
LED_1()��LED_2()��LED_3()��LED_4() �ֱ����˲�ͬ��LED��תЧ����
ÿ������ͨ������ P00��P01 �� P02 ���ŵ�״̬��ʵ�ֲ�ͬ����תЧ����
�жϷ������
Int0_isr() �жϷ�������ⰴ�����µ�״̬��
����������ʱ�������� ledstate ��ֵ�����л�����Ӧ��LED��תЧ����
��������
��ʼ���������жϡ�
���� ledstate ��ֵѡ����Ӧ��LED��תЧ������ִ�С�

Data��2024/10/19
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
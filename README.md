# 单片机课程设计实践说明
---
代码仅供参考学习交流使用，如代码侵犯你权利，请联系删除

开源协议为GPL3.0
如需商业使用需要对代码进行升级或维护服务，欢迎邮件[Laurie9k2b@gmail.com]联系[陳樂悅]，十分感谢喵 :3 !!!

如果在学习过程中对单片机中某功能不清楚其原理是什么，例如[IIC传输协议]，[Uart传输协议]，[PWM]相关等，可在[Discussions]中交流询问，虽说不一定会回消息。

# 项目列表
---

| 文件名称                    | 题目            | 要求                                                                               | 完成日期                             |
| ----------------------- | ------------- | -------------------------------------------------------------------------------- | -------------------------------- |
| 1_1Basic_RGB_LED        | 基础RGB流转灯      | 实现三种颜色轮流变化<br>变换间隔时间为500ms                                                       | 2024/10/19                       |
| 1_2Round_RGB_LED        | 手动RGB流转灯      | 可通过按键KEY1控制流转灯，按B-R-G顺序变化颜色，每次按键流转灯颜色变化一次，可循环实现，暗金相应实时性和稳定性好                     | 2024/10/19                       |
| 1_3Switch_Speed_RGB_LED | 变速RGB流转灯      | 流转灯初始间隔为500ms，通过按键KEY1改变流转灯的间隔，且满足间隔时间增加到2s后再次按键间隔时间恢复为初始值500ms的要求，按键相应应实时性和稳定性好 | 2024/10/19                       |
| 1_4Mult_RGB_LED         | 综合流转灯         | 实现基础流转灯和手动流转灯两种模式的切换，按键KEY1在基础模式下用于改变间隔时间，在手动模式下用于控制颜色变化                         | 2024/10/19                       |
| 2_1Temp_drive           | 电子温度计         | 利用温度传感器DS18B20和TM1650驱动的数码管模块实现温度检测和显示功能                                         | 2024/10/19                       |
| 2_2Ctrl_LED             | 多字节串口指令控制LED1 | 通过PC串口调试助手发送四字节控制指令，可实现对板载LED的控制                                                 | 2024/10/20                       |
| 2_3WS2812               | 多字节串口指令控制LED2 | 实现RGB独立LED控制和真彩LED控制两种功能                                                         | 2024/10/20                       |
| 3_1Ctrl_Fan             | 按键控制小风扇       | 通过按键改变小风扇的转速，转速分为停止、抵挡、中档和高档，转速差异明显，可循环切换                                        | 2024/10/20<br>（预计2024/10/26上传代码） |
| 3_2PC_Ctrl_Fan          | 串口控制小风扇       | 实现通过四字节串口指令控制风扇速度，风扇速度分为0~9档，0档不转，9档最快                                           | 2024/10/23 <br>（预计2024/10/26上传代码）      |
| 3_3Mult_Fan             | 多模式智能小风扇      | 实际具有按键调速、串口调速和温控调速三种模式，各模式功能实现完整，界面友好，操作简便，逻辑完整，具备操作可行性，在实际应用场景中具有一定贴合度          | 2024/10/24<br>（预计2024/10/26上传代码）      |

求求家人们点一个免费的Star吧，喵~~~

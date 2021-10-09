
/* 
**
** 
// Copyright (c) 2017, Pawpaw Electronic Technology., Ltd, All rights reserved
// This software is used for Pawpaw Microphone Array Source Project
// Visit at <http://www.pawpaw.hk/> for more information
**
*/

#ifndef _AUDIOCONFIG_H_
#define _AUDIOCONFIG_H_



#define DATA_TEST          (1)                // 播放测试数据是否打开，默认设置为1，打开用来测试段音乐的播放

#define SAMPLE_RATE        (16000)            // 采样率设置
#define FRAMES_PER_BUFFER  (256)              // buffer框架设计

/************************************************************************************************
 * 此处根据实际的选择设备的通道数目，进行定义，在这里以XMOS的板子来看
 * 输入10通道，输出2通道
 ************************************************************************************************/
#define INPUTCHANNEL   (18)                   // 输入设备的通道数目
#define OUTPUTCHANNEL  (2)                    // 输出设备的通道数目


#endif
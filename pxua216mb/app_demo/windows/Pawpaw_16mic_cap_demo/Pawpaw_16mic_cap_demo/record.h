/* 
**
** 
// Copyright (c) 2017, Pawpaw Electronic Technology., Ltd, All rights reserved
// This software is used for Pawpaw Microphone Array Source Project
// Visit at <http://www.pawpaw.hk/> for more information
**
*/
#ifndef _RECORD_H_
#define _RECORD_H_

#include "audioConfig.h"


typedef struct
{
	 /* 如果需要带入用户参数到录音回调函数中，请在此处定义*/
	 /* 这里为进行相关的测试，设计了一个数组用来接收，10个通道的信息*/
#if (DATA_TEST == 1) 
    short input_data[18];
#endif
}
paintputData;

extern int recordCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData );


#endif
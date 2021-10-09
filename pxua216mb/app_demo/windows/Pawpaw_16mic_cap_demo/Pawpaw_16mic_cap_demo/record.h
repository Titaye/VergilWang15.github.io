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
	 /* �����Ҫ�����û�������¼���ص������У����ڴ˴�����*/
	 /* ����Ϊ������صĲ��ԣ������һ�������������գ�10��ͨ������Ϣ*/
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
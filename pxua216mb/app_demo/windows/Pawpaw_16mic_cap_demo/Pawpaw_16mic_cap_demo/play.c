/* 
**
** 
// Copyright (c) 2017, Pawpaw Electronic Technology., Ltd, All rights reserved
// This software is used for Pawpaw Microphone Array Source Project
// Visit at <http://www.pawpaw.hk/> for more information
**
*/
#include "portaudio.h"
#include "audioConfig.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>


extern FILE *read_file;

/***************************************************************************************************************************************************
 * 功能：播放回调函数，当缓冲区一空，这个函数会被调用，可以从该函数中以-帧形式向输出缓冲写数据
 * 参数：
 * inputBuffer：录音数据缓冲地址，单纯的播放回调中，该参数一般设置为NULL
 * outputBuffer：播放数据缓冲地址，
 * framesPerBuffer：缓冲区的框架数，该值可通过外部设置，下面是详细的框架分布说明
                    每次进入回调的函数，需要写入buffer总的框架数，以现在这个程序来看，buffer中有256个框架，通俗的说
					就是有256帧数据，每一帧的通道数据的分布如下，以现在板子10通道进行说明：
					               -----------------------------------------------------------------------------------------------------------------
	                 buffer(256帧) - 第1帧-第2帧-第3帧-................................................................................-第256帧    -
					               -----------------------------------------------------------------------------------------------------------------
                                   -----------------------------------------------------------------------------------------------------------------
					 每一帧数据    - 通道1数据--通道2数据--通道3数据--通道4数据--通道5数据--通道6数据--通道7数据--通道8数据--通道9数据--通道10数据--
								   -----------------------------------------------------------------------------------------------------------------
                    
 * timeInfo：回调的时间信息
 * PaStreamCallbackFlags：回调标志
 * userData：用户数据指针，需要带入回调的用户参数
 *
 *************************************************************************************************************************************************/

int patestCallback( const void *inputBuffer, void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData )
{
    short *out = (short*)outputBuffer;
    unsigned long i;

    (void) timeInfo; /* Prevent unused variable warnings. */
    (void) statusFlags;
    (void) inputBuffer;

//	read_file = fopen("bingyu.pcm", "rb");
    
    for( i=0; i<framesPerBuffer; i++ )
    {
#if (DATA_TEST == 1) 
		fread(out++, 1, sizeof(short), read_file);
		fread(out++, 1, sizeof(short), read_file);

/*
		*out++ = *(ouput+count);
		count++;
		*out++ = *(ouput+count);
		count++;
		if(count == 7333260)
		{
			count = 0;
		}
*/
#endif
	    
    }
    
    return paContinue;
}
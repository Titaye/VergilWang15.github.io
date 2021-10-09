/* 
**
** 
// Copyright (c) 2017, Pawpaw Electronic Technology., Ltd, All rights reserved
// This software is used for Pawpaw Microphone Array Source Project
// Visit at <http://www.pawpaw.hk/> for more information
**
*/
#include "portaudio.h"
#include "record.h"
#include "audioConfig.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

extern FILE *file[4];
/***************************************************************************************************************************************************
 * 功能：录音回调函数，当缓冲区一满，这个函数会被调用，可以从该函数中以-帧拿到录音的数据
 * 参数：
 * inputBuffer：录音数据缓冲地址
 * outputBuffer：播放数据缓冲地址，单纯的播放回调中，该参数一般设置为NULL
 * framesPerBuffer：缓冲区的框架数，该值可通过外部设置，下面是详细的框架分布说明
                    每次进入回调的函数，可以拿出buffer总的框架数，以现在这个程序来看，buffer中有256个框架，通俗的说
					就是有256帧数据，每一帧的通道数据的分布如下，以现在板子10通道进行说明：
					               -----------------------------------------------------------------------------------------------------------------
	                 buffer(256帧) -                                                                                                               -
					               -----------------------------------------------------------------------------------------------------------------
                                   -----------------------------------------------------------------------------------------------------------------
					 每一帧数据    - 通道1数据--通道2数据--通道3数据--通道4数据--通道5数据--通道6数据--通道7数据--通道8数据--通道9数据--通道10数据--
								   -----------------------------------------------------------------------------------------------------------------
                    
 * timeInfo：回调的时间信息
 * PaStreamCallbackFlags：回调标志
 * userData：用户数据指针，需要带入回调的用户参数
 *
 *************************************************************************************************************************************************/
int recordCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    paintputData *data = (paintputData*)userData;
    const short *rptr = (const short*)inputBuffer;
    long i;
	int j;
    int finished = 0;

    (void) outputBuffer; /* Prevent unused variable warnin gs. */
    (void) timeInfo;
    (void) statusFlags;
    (void) userData;

	file[0] = fopen("xmos_record1.pcm", "ab");
	file[1] = fopen("xmos_record2.pcm", "ab");
	file[2] = fopen("xmos_record3.pcm", "ab");
	file[3] = fopen("xmos_record4.pcm", "ab");


	for( i=0; i<framesPerBuffer; i++ )
	{
#if (DATA_TEST == 1) 
		for(j =0; j < 18; j++)
		{
			data->input_data[j] = *rptr++;
		}
		fwrite(&(data->input_data[0]), 1, sizeof(short), file[0]);
		fwrite(&(data->input_data[1]), 1, sizeof(short), file[1]);
		fwrite(&(data->input_data[2]), 1, sizeof(short), file[2]);
		fwrite(&(data->input_data[3]), 1, sizeof(short), file[3]);
#endif
	}
	fclose(file[0]);
	fclose(file[1]);
	fclose(file[2]);
	fclose(file[3]);
    return finished;
}
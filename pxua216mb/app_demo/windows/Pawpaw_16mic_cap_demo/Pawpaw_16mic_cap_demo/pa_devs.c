/* 
**
** 
// Copyright (c) 2017, Pawpaw Electronic Technology., Ltd, All rights reserved
// This software is used for Pawpaw Microphone Array Source Project
// Visit at <http://www.pawpaw.hk/> for more information
**
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <math.h>
#include "portaudio.h"
#include "init_config.h"
#include "record.h"
#include "play.h"
#include "audioConfig.h"


FILE *file[4];
FILE *read_file;

int main(void)
{
    PaStreamParameters outputParameters,inputParameters;// input和output输出参数
    PaStream *outaudiostream;
	PaStream *inputaudiostream;
    PaError err;
    paoutputData outputdata;   // 定义带入播放回调函数的数据，这个数据可以用户自行定义，在play.h文件当中
	paintputData inputdata;    // 定义带入录音回调函数的数据，这个数据可以用户自行定义，在record.h文件当中
	char c;                    // 字符输入变量
	int inputdeviceid,outputdeviceid;   // 录音设备和播放设备的ID变量
	unsigned short ledValue;



	/* 添加文件 */
	file[0] = fopen("xmos_record1.pcm", "wb");
	file[1] = fopen("xmos_record2.pcm", "wb");
	file[2] = fopen("xmos_record3.pcm", "wb");
	file[3] = fopen("xmos_record4.pcm", "wb");
	read_file = fopen("bingyu.pcm", "rb");

	fclose(file[0]);
	fclose(file[1]);
	fclose(file[2]);
	fclose(file[3]);

    err = Pa_Initialize();     // 初始化Pa模块
    if( err != paNoError ) goto error;

	pa_device_select(&outputdeviceid,&inputdeviceid);        //选择设备进行录音播放

	inputParameters.device = inputdeviceid;                  // 设置录音设备ID号
    if (inputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default input device.\n");
        goto error;
    }

	// 输入设备参数设置
    inputParameters.channelCount = INPUTCHANNEL;            // 设置输入通道数
    inputParameters.sampleFormat = paInt16;                 // 设置录音格式
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

	// 输出设备参数设置
    outputParameters.device = outputdeviceid;               // 设置播放设备ID号
    if (outputParameters.device == paNoDevice) {
      fprintf(stderr,"Error: No default output device.\n");
      goto error;
    }
    outputParameters.channelCount = OUTPUTCHANNEL;          // 设置播放通道数
    outputParameters.sampleFormat = paInt16;                // 设置播放格式
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

	// 打开输出流
    err = Pa_OpenStream(
              &outaudiostream,
              NULL, /* no input */
              &outputParameters,
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,      
              patestCallback,
              &outputdata );
    if( err != paNoError ) goto error;

	// 打开输入流
    err = Pa_OpenStream(
              &inputaudiostream,
              &inputParameters,
              NULL,                  
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,      
              recordCallback,
              &inputdata );
    if( err != paNoError ) goto error;

    err = Pa_StartStream( outaudiostream );   // 开始播放流
    if( err != paNoError ) goto error;

	err = Pa_StartStream( inputaudiostream ); // 开始输入流
    if( err != paNoError ) goto error;

	while(1)
	{
        printf("/                  功能选择                            /\n");
		printf("/     1----播放暂停测试                                /\n");
		rewind(stdin);
		c = getchar();

		if(c == '1')
		{
			while(1)
			{
				printf("欢迎进入播放暂停测试\n");
				printf("a--暂停     b--播放    q--退出播放暂停测试\n");
				rewind(stdin);
				c = getchar();
				if(c == 'a')
				{
					printf("进入暂停状态\n");
					err = Pa_StopStream( outaudiostream );
					if( err != paNoError ) goto error;

				}else if(c == 'b')
				{
					printf("进入播放状态\n");
					err = Pa_StartStream( outaudiostream );
					if( err != paNoError ) goto error;
				}else if(c == 'q')
				{
					printf("退出播放暂停测试\n\n");
					break;
				}
			}

		}
		else
		{
			printf("输入错误！请输入字符1，2\n");
		}
				
	}

    Pa_Terminate();
    printf("Test finished.\n");
    
    return err;
error:
    Pa_Terminate();
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
    return err;
}

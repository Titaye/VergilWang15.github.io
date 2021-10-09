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
 * ���ܣ�¼���ص���������������һ������������ᱻ���ã����ԴӸú�������-֡�õ�¼��������
 * ������
 * inputBuffer��¼�����ݻ����ַ
 * outputBuffer���������ݻ����ַ�������Ĳ��Żص��У��ò���һ������ΪNULL
 * framesPerBuffer���������Ŀ��������ֵ��ͨ���ⲿ���ã���������ϸ�Ŀ�ֲܷ�˵��
                    ÿ�ν���ص��ĺ����������ó�buffer�ܵĿ�������������������������buffer����256����ܣ�ͨ�׵�˵
					������256֡���ݣ�ÿһ֡��ͨ�����ݵķֲ����£������ڰ���10ͨ������˵����
					               -----------------------------------------------------------------------------------------------------------------
	                 buffer(256֡) -                                                                                                               -
					               -----------------------------------------------------------------------------------------------------------------
                                   -----------------------------------------------------------------------------------------------------------------
					 ÿһ֡����    - ͨ��1����--ͨ��2����--ͨ��3����--ͨ��4����--ͨ��5����--ͨ��6����--ͨ��7����--ͨ��8����--ͨ��9����--ͨ��10����--
								   -----------------------------------------------------------------------------------------------------------------
                    
 * timeInfo���ص���ʱ����Ϣ
 * PaStreamCallbackFlags���ص���־
 * userData���û�����ָ�룬��Ҫ����ص����û�����
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
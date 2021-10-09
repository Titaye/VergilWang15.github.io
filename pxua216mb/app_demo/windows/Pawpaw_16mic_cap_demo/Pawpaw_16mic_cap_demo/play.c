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
 * ���ܣ����Żص���������������һ�գ���������ᱻ���ã����ԴӸú�������-֡��ʽ���������д����
 * ������
 * inputBuffer��¼�����ݻ����ַ�������Ĳ��Żص��У��ò���һ������ΪNULL
 * outputBuffer���������ݻ����ַ��
 * framesPerBuffer���������Ŀ��������ֵ��ͨ���ⲿ���ã���������ϸ�Ŀ�ֲܷ�˵��
                    ÿ�ν���ص��ĺ�������Ҫд��buffer�ܵĿ�������������������������buffer����256����ܣ�ͨ�׵�˵
					������256֡���ݣ�ÿһ֡��ͨ�����ݵķֲ����£������ڰ���10ͨ������˵����
					               -----------------------------------------------------------------------------------------------------------------
	                 buffer(256֡) - ��1֡-��2֡-��3֡-................................................................................-��256֡    -
					               -----------------------------------------------------------------------------------------------------------------
                                   -----------------------------------------------------------------------------------------------------------------
					 ÿһ֡����    - ͨ��1����--ͨ��2����--ͨ��3����--ͨ��4����--ͨ��5����--ͨ��6����--ͨ��7����--ͨ��8����--ͨ��9����--ͨ��10����--
								   -----------------------------------------------------------------------------------------------------------------
                    
 * timeInfo���ص���ʱ����Ϣ
 * PaStreamCallbackFlags���ص���־
 * userData���û�����ָ�룬��Ҫ����ص����û�����
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
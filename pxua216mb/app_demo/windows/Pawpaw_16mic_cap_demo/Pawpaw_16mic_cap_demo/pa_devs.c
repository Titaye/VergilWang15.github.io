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
    PaStreamParameters outputParameters,inputParameters;// input��output�������
    PaStream *outaudiostream;
	PaStream *inputaudiostream;
    PaError err;
    paoutputData outputdata;   // ������벥�Żص����������ݣ�������ݿ����û����ж��壬��play.h�ļ�����
	paintputData inputdata;    // �������¼���ص����������ݣ�������ݿ����û����ж��壬��record.h�ļ�����
	char c;                    // �ַ��������
	int inputdeviceid,outputdeviceid;   // ¼���豸�Ͳ����豸��ID����
	unsigned short ledValue;



	/* ����ļ� */
	file[0] = fopen("xmos_record1.pcm", "wb");
	file[1] = fopen("xmos_record2.pcm", "wb");
	file[2] = fopen("xmos_record3.pcm", "wb");
	file[3] = fopen("xmos_record4.pcm", "wb");
	read_file = fopen("bingyu.pcm", "rb");

	fclose(file[0]);
	fclose(file[1]);
	fclose(file[2]);
	fclose(file[3]);

    err = Pa_Initialize();     // ��ʼ��Paģ��
    if( err != paNoError ) goto error;

	pa_device_select(&outputdeviceid,&inputdeviceid);        //ѡ���豸����¼������

	inputParameters.device = inputdeviceid;                  // ����¼���豸ID��
    if (inputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default input device.\n");
        goto error;
    }

	// �����豸��������
    inputParameters.channelCount = INPUTCHANNEL;            // ��������ͨ����
    inputParameters.sampleFormat = paInt16;                 // ����¼����ʽ
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

	// ����豸��������
    outputParameters.device = outputdeviceid;               // ���ò����豸ID��
    if (outputParameters.device == paNoDevice) {
      fprintf(stderr,"Error: No default output device.\n");
      goto error;
    }
    outputParameters.channelCount = OUTPUTCHANNEL;          // ���ò���ͨ����
    outputParameters.sampleFormat = paInt16;                // ���ò��Ÿ�ʽ
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

	// �������
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

	// ��������
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

    err = Pa_StartStream( outaudiostream );   // ��ʼ������
    if( err != paNoError ) goto error;

	err = Pa_StartStream( inputaudiostream ); // ��ʼ������
    if( err != paNoError ) goto error;

	while(1)
	{
        printf("/                  ����ѡ��                            /\n");
		printf("/     1----������ͣ����                                /\n");
		rewind(stdin);
		c = getchar();

		if(c == '1')
		{
			while(1)
			{
				printf("��ӭ���벥����ͣ����\n");
				printf("a--��ͣ     b--����    q--�˳�������ͣ����\n");
				rewind(stdin);
				c = getchar();
				if(c == 'a')
				{
					printf("������ͣ״̬\n");
					err = Pa_StopStream( outaudiostream );
					if( err != paNoError ) goto error;

				}else if(c == 'b')
				{
					printf("���벥��״̬\n");
					err = Pa_StartStream( outaudiostream );
					if( err != paNoError ) goto error;
				}else if(c == 'q')
				{
					printf("�˳�������ͣ����\n\n");
					break;
				}
			}

		}
		else
		{
			printf("��������������ַ�1��2\n");
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

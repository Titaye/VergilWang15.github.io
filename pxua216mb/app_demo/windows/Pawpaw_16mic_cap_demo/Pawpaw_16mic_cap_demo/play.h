/* 
**
** 
// Copyright (c) 2017, Pawpaw Electronic Technology., Ltd, All rights reserved
// This software is used for Pawpaw Microphone Array Source Project
// Visit at <http://www.pawpaw.hk/> for more information
**
*/
#ifndef _PLAY_H_
#define _PLAY_H_

#include "portaudio.h"

#define TABLE_SIZE   (200)

typedef struct
{
    /* �����Ҫ�����û����������Żص������У����ڴ˴�����*/
	short output_data[2];
}
paoutputData; // paoutputData

int patestCallback( const void *inputBuffer, void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData );




#endif
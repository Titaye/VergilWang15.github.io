/* 
**
** 
// Copyright (c) 2017, Pawpaw Electronic Technology., Ltd, All rights reserved
// This software is used for Pawpaw Microphone Array Source Project
// Visit at <http://www.pawpaw.hk/> for more information
**
*/
#include <stdio.h>
#include <windows.h>
#include "portaudio.h"
#include "audioConfig.h"
#include "string.h"



/***************************************************************************************
 * 功能：打印设备列表，通常在一台PC上可能存在多个声卡设备，该函数打印出PC上所有的输入输出声卡
 * 参数：空
 * 
 ***************************************************************************************/
wchar_t wideName[MAX_PATH];
const PaHostApiInfo *hostInfo;

static void  pa_device_list()
{
	int     i,numDevices, defaultDisplayed;
	const   PaDeviceInfo *deviceInfo;
	PaError err;
	char input_device_flag = 0;
	char output_device_flag = 0;

   	numDevices = Pa_GetDeviceCount();
    if( numDevices < 0 )
    {
        printf( "ERROR: Pa_GetDeviceCount returned 0x%x\n", numDevices );
        err = numDevices;
    }
	printf( "Number of devices = %d\n", numDevices );


	for( i=0; i<numDevices; i++ )
	{
		deviceInfo = Pa_GetDeviceInfo(i);

		if(strstr(deviceInfo->name,"XS1-U8"))
		{
			if(deviceInfo->maxInputChannels == INPUTCHANNEL)
			{
                const PaHostApiInfo *hostInfo = Pa_GetHostApiInfo( deviceInfo->hostApi );
				if(strstr(hostInfo->name,"WASAPI"))
				{
					input_device_flag = 1;
					printf( "the input device = %d  --------", i );
					MultiByteToWideChar(CP_ACP, 0, deviceInfo->name, -1, wideName, MAX_PATH-1);
					wprintf( L" = %s\n", wideName );
				}
			}
			if(deviceInfo->maxOutputChannels == OUTPUTCHANNEL)
			{
				const PaHostApiInfo *hostInfo = Pa_GetHostApiInfo( deviceInfo->hostApi );
				if(strstr(hostInfo->name,"WASAPI"))
				{
					output_device_flag = 1;
					printf( "the output device = %d  --------", i );
					MultiByteToWideChar(CP_ACP, 0, deviceInfo->name, -1, wideName, MAX_PATH-1);
					wprintf( L" = %s\n", wideName );
				}
			}
		}
	}  

	if((output_device_flag == 0) && (input_device_flag == 0))
	{
		printf( "the device cannot find \n" );
	}

	if((output_device_flag == 1) && (input_device_flag == 1))
	{
		printf( "the device  find \n" );
	}



#if 0

	for( i=0; i<numDevices; i++ )
    {
		deviceInfo = Pa_GetDeviceInfo(i);
		printf( "--------------------------------------- device #%d\n", i );
        hostInfo = Pa_GetHostApiInfo( deviceInfo->hostApi );
        printf( "[ Default %s Input", hostInfo->name );
		MultiByteToWideChar(CP_ACP, 0, deviceInfo->name, -1, wideName, MAX_PATH-1);
        wprintf( L"Name                        = %s\n", wideName );
	}  
#endif
}

/***************************************************************************************
 * 功能：选择需要需要进行录音的声卡，获得输入设备和输出设备的ID号
 * 参数：指针变量
 *       *playdevID ： 播放设备的ID号
 *       *recdevID：   录音设备的ID号
 *       
 *
 ***************************************************************************************/
int pa_device_select( int *playdevID,int *recdevID)
{
	int inputdeviceid,outputdeviceid;
    pa_device_list();
	printf("please input inputdevice id.\n");fflush(stdout);
	rewind(stdin);
	scanf("%d",&inputdeviceid);
	printf("please input outputdevice id.\n");fflush(stdout);
	rewind(stdin);
	scanf("%d",&outputdeviceid);
	*playdevID = outputdeviceid;
	*recdevID = inputdeviceid;

}

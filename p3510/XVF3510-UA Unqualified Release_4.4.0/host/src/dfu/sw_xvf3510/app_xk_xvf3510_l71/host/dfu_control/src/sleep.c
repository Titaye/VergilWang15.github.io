// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include "sleep.h"
#if defined(_MSC_VER)
#include <Winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#endif

void sleep_milliseconds(unsigned milliseconds)
{
#if defined(_MSC_VER)
  Sleep(milliseconds);
#else
  usleep(milliseconds * 1000);
#endif
}

// Copyright (c) 2020, XMOS Ltd, All rights reserved
#ifndef __crc_h__
#define __crc_h__

unsigned crc_init(void);
void crc_step(unsigned *crc, unsigned word);
unsigned crc_finish(unsigned crc);

#endif

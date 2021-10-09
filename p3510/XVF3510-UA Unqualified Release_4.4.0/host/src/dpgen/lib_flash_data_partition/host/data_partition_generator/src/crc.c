// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include <stdio.h>
#include <stdbool.h>
#include "crc.h"

extern bool verbose;

unsigned crc_init(void)
{
  if (verbose)
    printf("CRC init\n");

  return 0xFFFFFFFF;
}

void crc_step(unsigned *crc, unsigned word)
{
  if (verbose)
    printf("CRC step 0x%X", word);

  for (int b = 0; b < 32; b++) {
    bool xor_bit = (*crc & 1) == 1;
    *crc = (*crc >> 1) | ((word & 1) << 31);
    word >>= 1;
    if (xor_bit)
      *crc ^= 0xEDB88320;
  }

  if (verbose)
    printf(" -> 0x%X\n", *crc);
}

unsigned crc_finish(unsigned crc)
{
  if (verbose)
    printf("CRC finish 0x%X\n", ~crc);

  return ~crc;
}

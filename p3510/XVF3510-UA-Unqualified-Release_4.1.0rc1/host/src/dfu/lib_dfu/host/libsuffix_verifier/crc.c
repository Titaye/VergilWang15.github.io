// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include <stdio.h>
#include <stdbool.h>
#include "crc.h"

static bool verbose = false;

unsigned crc_init(void)
{
  if (verbose)
    fprintf(stderr, "CRC init\n");

  return 0xFFFFFFFF;
}

void crc_step(unsigned *crc, unsigned char byte)
{
  if (verbose)
    fprintf(stderr, "CRC step 0x%X", byte);

  for (int b = 0; b < 8; b++) {
    bool xor_bit = (*crc & 1) == 1;
    *crc = (*crc >> 1) | ((unsigned)(byte & 1) << 31);
    byte >>= 1;
    if (xor_bit)
      *crc ^= 0xEDB88320;
  }

  if (verbose)
    fprintf(stderr, " -> 0x%X\n", *crc);
}

unsigned crc_finish(unsigned crc)
{
  if (verbose)
    fprintf(stderr, "CRC finish 0x%X\n", ~crc);

  return ~crc;
}

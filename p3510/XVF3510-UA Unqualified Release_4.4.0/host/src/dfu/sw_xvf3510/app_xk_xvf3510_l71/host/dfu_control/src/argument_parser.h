// Copyright (c) 2020, XMOS Ltd, All rights reserved
#ifndef __argument_parser_h__
#define __argument_parser_h__

#ifdef __XC__
#define _Bool int
#endif
#include <stdbool.h>
#include "device_id.h"

#define PRODUCT_ID_DEFAULT 0x0014
#define VENDOR_ID_DEFAULT 0x20B1
#define BCD_DEVICE_DEFAULT 0xFFFF // default 0xFFFF means skip checking
#define I2C_ADDRESS_DEFAULT 0x2C
#define BLOCK_SIZE_DEFAULT 128

extern bool quiet;

struct options {
  enum {
    UNKNOWN = 0,
    WRITE_UPGRADE,
    OVERRIDE_SPISPEC,
    DETACH_AND_BUS_RESET,
    REBOOT
  } operation;
  const char *arguments[2];
  struct device_id device_id;
  unsigned block_size;
  bool skip_boot_image;
  bool skip_data_image;
};

struct options parse_arguments(int argc, char **argv);

#endif

// Copyright (c) 2020, XMOS Ltd, All rights reserved
#ifndef __device_id_h__
#define __device_id_h__

#include <stdint.h>

struct device_id {
  uint16_t vendor, product, bcddevice;
  uint8_t i2c_address;
};

#endif

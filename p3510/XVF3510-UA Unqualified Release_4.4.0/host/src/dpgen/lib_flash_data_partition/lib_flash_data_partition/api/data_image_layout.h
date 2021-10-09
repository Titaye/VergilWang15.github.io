// Copyright (c) 2020, XMOS Ltd, All rights reserved
#ifndef __data_image_scanning__
#define __data_image_scanning__

#include <stdint.h>

// last item in a data image
#define DP_IMAGE_TYPE_TERMINATOR 1

// data image items are of type-length-value format
struct data_image_tlv_header {
  uint32_t type;
  uint32_t length;
};

#endif

// Copyright (c) 2020, XMOS Ltd, All rights reserved
#ifndef __inputs_h__
#define __inputs_h__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct images {
  struct image {
    bool in_use;
    struct version {
      int major, minor, patch;
    } compatibility_version;
    struct item {
      unsigned type;
      uint8_t *bytes;
      size_t num_bytes;
      struct item *next;
    } *item_list;
  } factory, upgrade;
};

struct spispec {
  bool valid;
  const char *file_name;
  unsigned char *bytes;
  size_t num_bytes;
};

#endif

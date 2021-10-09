// Copyright (c) 2020, XMOS Ltd, All rights reserved
#ifndef __renderer_h__
#define __renderer_h__

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "inputs.h"

struct byte_stream {
  uint8_t *bytes;
  size_t length;
};

struct byte_stream render_byte_stream(const struct images *images,
                                      unsigned hardware_build,
                                      const char *serial_number,
                                      struct spispec spispec,
                                      unsigned sector_size,
                                      bool bad_factory_crc,
                                      bool bad_upgrade_crc);

void cleanup_byte_stream(struct byte_stream stream);

#endif

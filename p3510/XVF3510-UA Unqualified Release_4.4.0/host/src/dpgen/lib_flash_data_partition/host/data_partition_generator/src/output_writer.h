// Copyright (c) 2020, XMOS Ltd, All rights reserved
#ifndef __output_writer_h__
#define __output_writer_h__

#include <stdbool.h>
#include "inputs.h"

void write_output(const struct images *images, unsigned hardware_build,
                  struct spispec spispec, unsigned sector_size,
                  bool bad_factory_crc, bool bad_upgrade_crc,
                  const char *serial_number, const char *out_file_name);

#endif

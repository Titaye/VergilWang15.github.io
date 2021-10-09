// Copyright (c) 2020, XMOS Ltd, All rights reserved
#ifndef __input_reader_h__
#define __input_reader_h__

#include "inputs.h"

struct images read_inputs(const char *factory_file_name,
                          const char *upgrade_file_name);

struct spispec read_flash_specification(const char *file_name);

void cleanup_images(struct images *images);

#endif

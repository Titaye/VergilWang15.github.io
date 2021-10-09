// Copyright (c) 2020, XMOS Ltd, All rights reserved
#ifndef __operations_h__
#define __operations_h__

#include <stdbool.h>
#include "input_reader.h"

int write_upgrade(struct inputs inputs, unsigned block_size,
                  bool skip_boot_image, bool skip_data_image);

int override_spispec(struct inputs inputs);

int detach_and_bus_reset(void);

#endif

// Copyright (c) 2020, XMOS Ltd, All rights reserved
#ifndef __hal_h__
#define __hal_h__

#include <stddef.h>
#include "dfu_commands.h"
#include "device_id.h"

int hal_connect(struct device_id device_id);

int hal_read_command(enum dfu_command command,
                     unsigned char *payload, size_t num_bytes);

int hal_write_command(enum dfu_command command,
                      const unsigned char *payload, size_t num_bytes);

int hal_reboot(void);

int hal_disconnect(void);

#endif

// Copyright (c) 2020, XMOS Ltd, All rights reserved
#ifndef __labels_h__
#define __labels_h__

#include "dfu_commands.h"
#include "dfu_types.h"

const char *command_str(enum dfu_command command);
const char *state_str(enum dfu_state state);
const char *status_str(enum dfu_status status);

#endif

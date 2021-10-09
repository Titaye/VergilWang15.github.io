// Copyright (c) 2017-2020, XMOS Ltd, All rights reserved
#ifndef __dfu_commands_h__
#define __dfu_commands_h__

#ifdef __xcore__
#include <platform.h>
#include <quadflash.h>
#endif
#include <stddef.h>
#include <stdint.h>
#ifdef __XC__
#define _Bool int
#endif
#include <stdbool.h>
#ifdef __xcore__
#include "control.h"
#endif
#include "control_transport.h"

#define RESOURCE_ID_DFU 0xD0

enum dfu_command {
  DFU_CMD_DETACH           = CONTROL_CMD_SET_WRITE(1),
  DFU_CMD_BUS_RESET        = CONTROL_CMD_SET_WRITE(2),
  DFU_CMD_DNLOAD           = CONTROL_CMD_SET_WRITE(3),
  DFU_CMD_CLRSTATUS        = CONTROL_CMD_SET_WRITE(4),
  DFU_CMD_REBOOT           = CONTROL_CMD_SET_WRITE(5),
  DFU_CMD_GETSTATE         = CONTROL_CMD_SET_READ(6),
  DFU_CMD_GETSTATUS        = CONTROL_CMD_SET_READ(7),
  DFU_CMD_GET_ERROR_INFO   = CONTROL_CMD_SET_READ(8),
  DFU_CMD_OVERRIDE_SPISPEC = CONTROL_CMD_SET_WRITE(9)
};

struct dfu_get_version {
  uint8_t major, minor, patch;
};

struct dfu_dnload_header {
  uint16_t block_num;
  uint16_t pad;
};

struct dfu_timeout {
  bool enable;
  unsigned delta;
};

struct dfu_write_command_state {
  struct dfu_timeout timeout;
  bool needs_reboot;
  bool needs_flash_connect;
  bool flash_connected;
};

#ifdef __XC__
control_ret_t dfu_handle_write_command(int cmd, const char payload[],
                                       size_t payload_len,
                                       struct dfu_write_command_state &state,
                                       fl_QuadDeviceSpec spispec[1]);

control_ret_t dfu_handle_read_command(int cmd, char payload[],
                                      size_t payload_len);
#endif

#endif

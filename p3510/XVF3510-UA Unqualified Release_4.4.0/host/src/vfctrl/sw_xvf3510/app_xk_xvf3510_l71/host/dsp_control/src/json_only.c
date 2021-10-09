// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include "control.h"
// Add mock lib_device_control functions
control_ret_t control_read_command(control_resid_t resid, control_cmd_t cmd,
                     uint8_t payload[], size_t payload_len) {
    return CONTROL_SUCCESS;
};
control_ret_t control_write_command(control_resid_t resid, control_cmd_t cmd,
                      const uint8_t payload[], size_t payload_len){
    return CONTROL_SUCCESS;
};
void host_shutdown(void) {};

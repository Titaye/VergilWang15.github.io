// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/time.h>
//#include <libusb-1.0/libusb.h>
#include <assert.h>
#include <linux/input.h>
#include <linux/input-event-codes.h>

#include "xmos_user_app.h"
#include "jni.h"
#include "android/log.h"
#include "host_control_api.h"

#define TAG "xmos_device_manager"


#include <stdlib.h>
#include "jni.h"
#include "android/log.h"
#include "libusb.h"
#include "control_host_support.h"
#define CMD_MAX_RET_STRING 1000

char ret_string[CMD_MAX_RET_STRING];

char* run_command(int num_args, const char** cmd) {
    memset(ret_string, 0, strlen(ret_string));
    __android_log_print(ANDROID_LOG_INFO, TAG,
                        "run %s", cmd[0]);
    if (strcasecmp(cmd[0], "--help")==0) {
        __android_log_print(ANDROID_LOG_INFO, TAG,
                            "print_help");
        char* cmd_list =  vfctrl_print_help(1);
        return cmd_list;
    }
    cmdspec_t cmd_spec;
    int ret = vfctrl_get_cmdspec(num_args, cmd[0], &cmd_spec, 0);
    if(ret != 0) {
        char* error = "";
        switch(ret) {
            case -1:
                error = "Command not found";
                break;
            case -2:
                error = "Wrong number of arguments";
                break;
            default:
                error = "Undefined error";
                break;
        }
        __android_log_print(ANDROID_LOG_INFO, TAG,
                            "%s", error);
        return error;
    }
    __android_log_print(ANDROID_LOG_INFO, TAG,
                        "allocate %d bytes", cmd_spec.app_read_result_size);

    char* result_data = malloc(cmd_spec.app_read_result_size);

    ret = vfctrl_do_command(&cmd_spec, cmd, result_data, 0);
    if (ret != 0) {
        char *error = "Command failed";
        __android_log_print(ANDROID_LOG_INFO, TAG,
                "%s", error);
        return error;
    }

    if (!strncmp(&cmd_spec.par_name, "GET_", strlen("GET_"))) {
         vfctrl_format_read_result(&cmd_spec, result_data, ret_string);
    } else {
        memcpy(ret_string, "Command success", strlen("Command success"));
    }
    return ret_string;
}



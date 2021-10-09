// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef HOST_CONTROL_API_H
#define HOST_CONTROL_API_H
#include <stdint.h>

typedef uint8_t control_resid_t;
#define MAX_PAR_NAME_CHARS (40)
#define MAX_PAR_INFO_CHARS (200)
#define CMD_MAX_BYTES      (64)

#define XVF3510_PID_DEFAULT (0x0014)
#define XVF3510_VID_DEFAULT (0x20b1)

typedef enum {READ, WRITE} param_rw;

typedef enum {
    TYPE_FIXED_0_32,
    TYPE_FIXED_1_31,
    TYPE_FIXED_7_24,
    TYPE_FIXED_8_24,
    TYPE_FIXED_16_16,
    TYPE_UINT8,
    TYPE_INT8,
    TYPE_UINT32,
    TYPE_INT32,
    TYPE_UINT64,
    TYPE_INT64,
    TYPE_TICKS,
    TYPE_ENERGY,
    TYPE_NA
} param_type;

typedef struct cmdspec_t {
  control_resid_t resid;
  char par_name[MAX_PAR_NAME_CHARS];
  param_type type;
  unsigned offset;
  param_rw rw;
  unsigned num_values; //no. of values read or written from the device
  char info[MAX_PAR_INFO_CHARS];
  unsigned device_rw_size; //no. of bytes per value read or written from the device for a given type.
  unsigned app_read_result_size; //for read commands, the total amount of memory app needs to allocate and send for returning read values into .
} cmdspec_t;

//API functions
void vfctrl_set_vendor_id(int vendor_id);
void vfctrl_set_product_id(int product_id);
char* vfctrl_print_help(unsigned full);
void vfctrl_dump_params(void);
int vfctrl_get_cmdspec(int num_args, const char *command, cmdspec_t *cmd_spec, uint8_t log_for_data_partition);
int vfctrl_do_command(cmdspec_t *cmd_spec, const char **command_plus_values, void *data_out_ptr, uint8_t log_for_data_partition);
int vfctrl_get_aec_coefficients_to_file(const char* aec_coeffs_file);
int vfctrl_get_ic_coefficients_to_file(const char* aec_coeffs_file);
int vfctrl_get_filter_coefficients_human_readable(cmdspec_t *cmd_original);
int vfctrl_set_filter_coefficients_human_readable(cmdspec_t *cmd_original, const char **command_plus_values, unsigned log_for_data_partition);
int vfctrl_format_read_result(cmdspec_t *cmd_spec_ptr, void* data_out_ptr, char* output_string);
int vfctrl_check_version(unsigned print_version_only);
int vfctrl_check_run_status();

#endif


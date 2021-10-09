// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "host_control_api.h"
#include <math.h>

#define MAX_NUM_ARGUMENTS (100)
#define OUTPUT_STR_MAX_CHARS  (1000)

int main(int argc, char **argv)
{
    int ret=0;

    uint8_t do_version_check = 1;
#if JSON_ONLY
    uint8_t log_for_data_partition = 1;
#else
    uint8_t log_for_data_partition = 0;
#endif
    if (argc>MAX_NUM_ARGUMENTS) {
        printf("Error: Too many arguments, %d found and max number is %d\n", argc, MAX_NUM_ARGUMENTS);
        exit(1);
    }
    vfctrl_set_vendor_id(XVF3510_VID_DEFAULT);
    vfctrl_set_product_id(XVF3510_PID_DEFAULT);
    char* final_argv[MAX_NUM_ARGUMENTS];
    int final_argc = 0;
    // look for optional arguments and store the other arguments in the reduce argument list
    for (int arg_idx=0; arg_idx<argc; arg_idx++) {
        if ( (strcmp(argv[arg_idx], "--no-check-version") == 0 ) || (strcmp(argv[arg_idx], "-n") == 0) ) {
            do_version_check = 0;
            continue;

        }
        if ( (strcmp(argv[arg_idx], "--log-data-partition") == 0 ) || (strcmp(argv[arg_idx], "-l") == 0) ) {
            log_for_data_partition = 1;
            continue;
        }
        if ( ( (strcmp(argv[arg_idx], "--product-id") == 0)  || (strcmp(argv[arg_idx], "-p") == 0 ) ) && arg_idx + 1 <= argc - 1 ) {
            int product_id = strtol(argv[arg_idx + 1], NULL, 0);
            printf("Setting product id 0x%04x\n", product_id);
            vfctrl_set_product_id(product_id);
            arg_idx++;
            continue;
        }
        if ( ( (strcmp(argv[arg_idx], "--vendor-id") == 0 ) || (strcmp(argv[arg_idx], "-v") == 0 ) ) && arg_idx + 1 <= argc - 1 ) {
            int vendor_id = strtol(argv[arg_idx + 1], NULL, 0);
            printf("Setting vendor id 0x%04x\n", vendor_id);
            vfctrl_set_vendor_id(vendor_id);
            arg_idx++;
            continue;
        }
        // all the arguments not parsed above will be part of the final argument list
        final_argv[final_argc] = argv[arg_idx];
        final_argc++;
    }

    if (final_argc < 2) {
        vfctrl_print_help(0);
        vfctrl_check_version(1);
        exit(0);
    }
    if (final_argc == 2 && (strcmp(final_argv[1], "--help") == 0 || strcmp(final_argv[1], "-h") == 0)) {
        vfctrl_print_help(1);
        vfctrl_check_version(1);
        exit(0);
    }
    if (final_argc == 2 && (strcmp(final_argv[1], "--dump-params") == 0 || strcmp(final_argv[1], "-d") == 0)) {
        vfctrl_dump_params();
        exit(0);
    }

    cmdspec_t cmd_spec;
    ret = vfctrl_get_cmdspec(final_argc-1, final_argv[1], &cmd_spec, log_for_data_partition);
    if(ret != 0)
    {
        exit(ret);
    }

    if (do_version_check && !log_for_data_partition) {
        ret = vfctrl_check_version(0);
        if (ret) {
            printf("Error: Cannot read device version\n");
        }
    }

    if (!log_for_data_partition) {
        cmdspec_t cmd;;

        int ret = vfctrl_get_cmdspec(1, "GET_RUN_STATUS", &cmd, 0);
        if(ret != 0)
        {
            exit(ret);
        }
        ret = vfctrl_check_run_status(cmd);
        if (ret) {
            printf("Error: Cannot read run status\n");
        }
    }

    //sanity check final_argv
    if(cmd_spec.rw == WRITE)
    {
        assert(cmd_spec.num_values == (final_argc - 2));
    }

    //allocate memory for do_command
    void* data_out_ptr = calloc(cmd_spec.app_read_result_size, 1);
    char* output_string = calloc(OUTPUT_STR_MAX_CHARS, 1);

    if (strcmp("GET_FILTER_COEFFICIENTS_AEC", cmd_spec.par_name) == 0) {
        ret = vfctrl_get_aec_coefficients_to_file("aec_coefficients.py");
    } else if (strcmp("GET_FILTER_COEFFICIENTS_IC", cmd_spec.par_name) == 0) {
        ret = vfctrl_get_ic_coefficients_to_file("ic_coefficients.py");
    } else if (strcmp("GET_FILTER_COEFF", cmd_spec.par_name) == 0) {
        ret = vfctrl_get_filter_coefficients_human_readable(&cmd_spec);
    } else if (strcmp("SET_FILTER_COEFF", cmd_spec.par_name) == 0) {
        ret = vfctrl_set_filter_coefficients_human_readable(&cmd_spec, (const char**)&final_argv[1], log_for_data_partition);
    } else {
        ret = vfctrl_do_command(&cmd_spec, (const char**)&final_argv[1], data_out_ptr, log_for_data_partition);
        if(!ret && (cmd_spec.rw == READ)){
            vfctrl_format_read_result(&cmd_spec, data_out_ptr, output_string);
            printf("%s\n", output_string);
        }
    }
    if(ret != 0)
    {
        printf("vfctrl_do_command() returned error\n");
    }
    free(data_out_ptr);
    free(output_string);
    exit(ret);
}

// Copyright (c) 2018-2021, XMOS Ltd, All rights reserved
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdarg.h>
#include <assert.h>

#if !JSON_ONLY
#include <control_host.h>
#endif
#include "host_control_api.h"
#include "host_control.h"
#include <math.h>

#define MAX_INT32   (0x7FFFFFFF)
// For sleep functions
#include <time.h>
#if defined(_MSC_VER)
#define strcasecmp(x,y) _stricmp(x,y)
#include <Winsock2.h>
#pragma comment(lib, "ws2_32.lib")
HANDLE lock;
#define LOCK_MUTEX lock=CreateMutexW(NULL, TRUE, NULL);
#define UNLOCK_MUTEX ReleaseMutex(lock);

#else
#include <pthread.h>
pthread_mutex_t lock;
#define LOCK_MUTEX pthread_mutex_lock(&lock);
#define UNLOCK_MUTEX pthread_mutex_unlock(&lock);
#include <unistd.h>
#include <arpa/inet.h>
#ifdef __ANDROID__
#include <android/log.h>
#define TAG "host_app"
#endif // __ANDROID__

void Sleep(unsigned milliseconds)
{
    usleep(milliseconds*1000);
}
#endif

// htonll() and ntohll() are not available on same OS, for example Raspian
// Therefore the macros below redefine them if necessary. The macros will:
// - check endianness of the platform
// - and use htonl()/ntohl() to shift the bytes
#ifndef htonll
#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#endif
#ifndef ntohll
#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))
#endif

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#define MAX_SIMILAR_COMMANDS (20)

int g_product_id;
int g_vendor_id;


#define TEMP_STR_MAX_CHARS  (1000)
#define CMD_LIST_MAX_CHARS  (10000)


int total_num_commands;
cmdspec_t *cmdspec_ap = NULL;
int setup_err = 1;
char cmd_list[CMD_LIST_MAX_CHARS];

// number of bytes per line shown in the JSON data partition file
#define LOG_BYTES_PER_LINE (16)

void host_shutdown(int exit_code){
    #if USE_USB
    control_cleanup_usb();
    #elif USE_I2C
    control_cleanup_i2c();
    #endif
    UNLOCK_MUTEX
    exit(exit_code);
}

int get_size_from_type(param_type type)
{
    int ret_val = -1;
    switch (type) {
        case TYPE_INT8:
        case TYPE_UINT8:
            ret_val = 1;
            break;
        case TYPE_FIXED_0_32:
        case TYPE_FIXED_1_31:
        case TYPE_FIXED_7_24:
        case TYPE_FIXED_8_24:
        case TYPE_FIXED_16_16:
        case TYPE_INT32:
        case TYPE_UINT32:
        case TYPE_TICKS:
            ret_val = 4;
            break;
        case TYPE_ENERGY:
        case TYPE_INT64:
        case TYPE_UINT64:
            ret_val = 8;
            break;
        default:
            printf("Error: invalid type %d\n", type);
            host_shutdown(1);
    }
    return ret_val;
}

int get_app_read_result_size(param_type type)
{
    int ret_val = -1;
    switch (type) {
        case TYPE_INT8:
        case TYPE_UINT8:
            ret_val = sizeof(int8_t);
            break;
        case TYPE_ENERGY:
        case TYPE_FIXED_0_32:
        case TYPE_FIXED_1_31:
        case TYPE_FIXED_7_24:
        case TYPE_FIXED_8_24:
        case TYPE_FIXED_16_16:
            ret_val = sizeof(float);
            break;
        case TYPE_INT32:
        case TYPE_UINT32:
        case TYPE_TICKS:
            ret_val = sizeof(int32_t);
            break;
        case TYPE_INT64:
        case TYPE_UINT64:
            ret_val = sizeof(int64_t);
            break;
        default:
            printf("Error: invalid type %d\n", type);
            host_shutdown(1);
    }
    return ret_val;
}

cmdspec_t cmd_con(control_resid_t resid,
                  char par_name[MAX_PAR_NAME_CHARS],
                  param_type type,
                  unsigned offset,
                  param_rw rw,
                  unsigned num_values,
                  char info[MAX_PAR_INFO_CHARS])
{
    cmdspec_t cmd;
    cmd.resid = resid;
    strcpy(cmd.par_name, par_name);
    cmd.type = type;
    cmd.offset = offset;
    cmd.num_values = num_values;

    cmd.rw = rw;
    strcpy(cmd.info, info);
    cmd.device_rw_size = get_size_from_type(type);
    cmd.app_read_result_size = 0;
    if(cmd.rw == READ)
    {
        if((strcmp("GET_ERLE_CH0_AEC", par_name) == 0) || (strcmp("GET_ERLE_CH1_AEC", par_name) == 0))
        {
            //for ERLE commands, 2 values are read from device but only 1 returned to app
            cmd.app_read_result_size = get_app_read_result_size(type);
        }
        else
        {
            cmd.app_read_result_size = num_values * get_app_read_result_size(type);
        }
    }
    return cmd;
}

void print_set_io_map(int_float* vals)
{
    int max_value = sizeof(output_io_map_str)/sizeof(char*) - 1;
    if (vals[0].ui8 > max_value) {
        fprintf(stderr, "Error: first argument is invalid: max value %d, given value: %d\n", max_value, vals[0].ui8);
        host_shutdown(-1);
    }
    max_value = sizeof(input_io_map_str)/sizeof(char*) - 1;
    if (vals[1].ui8 > max_value) {
        fprintf(stderr, "Error: second argument is invalid: max value %d, given value: %d\n", max_value, vals[1].ui8);
        host_shutdown(-1);
    }

    printf("\n******* set_io_map %s %s ******\n\n", output_io_map_str[vals[0].ui8], input_io_map_str[vals[1].ui8]);
}

char* format_io_map(uint8_t *vals, char *io_map_string)
{
    sprintf(io_map_string, "\n");
    const char *shift_direction[2] = {"LEFT", "RIGHT"};

    for(unsigned i=0; i<NUM_IO_MAP_OUTPUTS; i++)
    {
        char temp_string[TEMP_STR_MAX_CHARS];
        if(vals[i*3] < NUM_IO_MAP_INPUTS)
        {
            sprintf(temp_string, "target: %s, source: %s ", output_io_map_str[i], input_io_map_str[vals[i*3]]);
            strcat(io_map_string, temp_string);
        }
        else
        {
            sprintf(temp_string, "target: %s, source: INVALID ", output_io_map_str[i]);
            strcat(io_map_string, temp_string);
        }
        if(vals[i*3 + 1] == 0)
        {
            sprintf(temp_string, "output shift: NONE");
            strcat(io_map_string, temp_string);
        }
        else {
            if(vals[i*3 + 2] < 2) {
                sprintf(temp_string, "target shift: %d bits to the %s", vals[i*3 + 1], shift_direction[vals[i*3 + 2]]);
                strcat(io_map_string, temp_string);
            }
            else {
                sprintf(temp_string, "target shift: %d bits in INVALID direction", vals[i*3 + 1]);
                strcat(io_map_string, temp_string);
            }
        }
        sprintf(temp_string, "\n");
        strcat(io_map_string, temp_string);
    }
    return io_map_string;
}

char* format_run_status(uint8_t *vals, char *run_status_string)
{
    strcat(run_status_string, " ");
    // remove RUN_ prefix
    strcat(run_status_string, run_status_str[vals[0]]+4);
    return run_status_string;
}


char* format_locker_state(uint32_t *vals, char *state_string)
{
    strcat(state_string, " ");

    if(vals[0] < NUM_LOCKER_STATES)
    {
        strcat(state_string, locker_state[vals[0]]);
    }
    else
    {
        strcat(state_string, " INVALID");
    }
    return state_string;
}


#define NUM_WRITE_RETRIES (10)
#define SCALE_Q(SHIFT) (unsigned) (1 << SHIFT)
// treat the 32-bit shift as a special case since it requires an uint64_t value
#define SCALE_Q_32    ((uint64_t) (1) << 32)

#if USE_I2C
    #define MAX_NUM_OF_INT_PER_TRANSFER ( I2C_DATA_MAX_BYTES/sizeof(unsigned) )
    int i2c_setup(void)
    {

        unsigned i2c_address = 0x2c;
        unsigned i2c_shift = 0;
        if (control_init_i2c(i2c_address << i2c_shift) != CONTROL_SUCCESS) { //Note on some RPI the I2C address needs left shifting by one
            fprintf(stderr, "Error: Control initialisation over I2C failed, address 0x%02x<<%d\n", i2c_address, i2c_shift);
            host_shutdown(-1);
        }

        unsigned char data[3];
        control_ret_t connected = control_read_command(CONTROL_SPECIAL_RESID, CONTROL_GET_VERSION, data, sizeof(control_version_t));
        if (connected != CONTROL_SUCCESS) {
            fprintf(stderr, "Error: Unable to find I2C device at address 0x%02x\n", i2c_address);
            host_shutdown(-1);
        }
        control_version_t version = data[0];

        if (version != CONTROL_VERSION) {
            fprintf(stderr, "Error: Mismatch of the control version between host and device. Expected 0x%X, received 0x%X\n", CONTROL_VERSION, version);
            host_shutdown(-1);
        }

        return 0;
    }
#elif USE_USB
    #define MAX_NUM_OF_INT_PER_TRANSFER ( USB_DATA_MAX_BYTES/sizeof(unsigned) )
    int usb_setup()
    {
        control_version_t version;

        if(control_init_usb(g_vendor_id, g_product_id, 3) != CONTROL_SUCCESS) {
            fprintf(stderr, "Error: Control initialisation over USB failed\n");
            host_shutdown(-1);
        }

        if (control_query_version(&version) != CONTROL_SUCCESS) {
            fprintf(stderr, "Error: Control query version failed\n");
            host_shutdown(-1);
        }

        if (version != CONTROL_VERSION) {
            fprintf(stderr, "Error: Mismatch of the control version between host and device. Expected 0x%X, received 0x%X\n", CONTROL_VERSION, version);
            host_shutdown(-1);
        }

        return 0;
    }
#elif JSON_ONLY
    // use function in json_only.c
#else
  #error "Need to specify which interface to use, e.g. USE_I2C"
#endif

int levenshtein_distance(const char str1[], const char str2[]) {
   int i, j, temp, track;
   int dist[MAX_PAR_NAME_CHARS][MAX_PAR_NAME_CHARS];
   int len1 = MIN(strlen(str1), MAX_PAR_NAME_CHARS);
   int len2= MIN(strlen(str2), MAX_PAR_NAME_CHARS);
   for(i=0; i<=len1; i++) {
      dist[0][i] = i;
   }
   for(j=0; j<=len2; j++) {
      dist[j][0] = j;
   }
   for (j=1; j<=len1; j++) {
      for(i=1; i<=len2; i++) {
         if(str1[i-1] == str2[j-1]) {
            track = 0;
         } else {
            track = 1;
         }
         temp = MIN((dist[i-1][j]+1), (dist[i][j-1]+1));
         dist[i][j] = MIN(temp, (dist[i-1][j-1]+track));
      }
   }
   return dist[len2][len1];
}
int get_cmdspec_num(cmdspec_t cmdspec[], int num_params, const  char* field)
{
    uint8_t min_dist = 0xFF;
    char min_dist_cmd[MAX_SIMILAR_COMMANDS][MAX_PAR_NAME_CHARS];
    memset(min_dist_cmd, 0, MAX_PAR_NAME_CHARS);
    int min_dist_cmd_idx = -1;
    char uppercase_field[MAX_PAR_NAME_CHARS] = {0};
    bool similar_cmd_found = false;
    for (uint32_t str_i=0; str_i<MIN(MAX_PAR_NAME_CHARS, strlen(field)); str_i++) {
        uppercase_field[str_i] = toupper(field[str_i]);
    }
    for(int i=0; i<num_params; i++) {
#ifdef __ANDROID__
        if(strcmp(cmdspec[i].par_name, field) == 0) {
            return i;
        }
#else
        uint8_t curr_dist = levenshtein_distance(uppercase_field, cmdspec[i].par_name);
        // if a new min distance is found, reset the min_dist_cmd list
        if (min_dist > curr_dist) {
            min_dist = curr_dist;
            min_dist_cmd_idx = 0;
            strcpy(min_dist_cmd[min_dist_cmd_idx], cmdspec[i].par_name);
        } else if (min_dist == curr_dist) {
            if (min_dist_cmd_idx < MAX_SIMILAR_COMMANDS) {
                min_dist_cmd_idx++;
                strcpy(min_dist_cmd[min_dist_cmd_idx], cmdspec[i].par_name);
            }
        }
        // if the Levenshtein distance is zero, the command is found
        if(curr_dist == 0) {
            return i;
        }
#endif // __ANDROID__
    }
    printf("Error: Command %s not found\n", field);

    // if the given string is too short, print the commands which include the string in their name
    // otherwise print the commands with minimum Levenshtein distance.
    if (strlen(uppercase_field)<6) {
        for(int i=0; i<num_params; i++) {
            if (strstr(cmdspec[i].par_name, uppercase_field) != NULL) {
                if (!similar_cmd_found) {
                    printf("Did you mean one of the commands below?\n");
                }
                similar_cmd_found = true;
                printf("    %s\n", cmdspec[i].par_name);
            }
        }
    } else {
        for (int i=0; i <= min_dist_cmd_idx; i++) {
            printf("    %s\n", min_dist_cmd[i]);
        }
    }
    return -1;
}

uint8_t is_same_string_ending(const char * const full_string, const char * const string_ending) {
    int full_string_len =  strlen(full_string);
    int string_ending_len = strlen(string_ending);
    uint8_t is_same_ending  = 0;
    if(full_string_len > string_ending_len) {
        // compare only the ending of the string
        is_same_ending = !strcmp(full_string + full_string_len - string_ending_len, string_ending);
    }
    return is_same_ending;
}

int write_payload_byte_array(cmdspec_t current, int num_values, int_float *ptr_data, uint8_t *payload)
{
    unsigned data_size = current.device_rw_size;
    switch(data_size) {
        case 1:
            for(int32_t i=0; i<num_values; i++)
            {
                payload[i] = ptr_data[i].ui8;
            }
        break;
        case 4:
            for(int32_t i=0; i<num_values; i++)
            {
                uint32_t t = htonl(ptr_data[i].i);
                memcpy(&payload[i*data_size], &t, data_size);
            }
        break;
        case 8:
            for(int32_t i=0; i<num_values; i++)
            {
                uint64_t t = htonll(ptr_data[i].i_long);
                memcpy(&payload[i*data_size], &t, data_size);
            }
        break;
        default:
            fprintf(stderr, "Error: %s: data_size %d not supported\n", __func__, data_size);
            return -1;
        break;
    }
    return 0;
}

int read_payload_byte_array(cmdspec_t current, int num_values, uint8_t *payload, int_float *ptr_data)
{
    unsigned data_size = current.device_rw_size;
    switch(data_size) {
        case 1:
            for(int32_t i=0; i<num_values; i++)
            {
                ptr_data[i].ui8 = payload[i];
            }
        break;
        case 4:
            for(int32_t i=0; i<num_values; i++)
            {
                uint32_t temp;
                memcpy(&temp, &payload[i*data_size], data_size);
                ptr_data[i].i = ntohl(temp);
            }
        break;
        case 8:
            for(int32_t i=0; i<num_values; i++)
            {
                uint64_t temp;
                memcpy(&temp, &payload[i*data_size], data_size);
                ptr_data[i].i_long = ntohll(temp);
            }
        break;
        default:
            fprintf(stderr, "Error: %s: data_size %d not supported\n", __func__, data_size);
            return -1;
        break;
    }
    return 0;
}

void print_log_partition_info(cmdspec_t current, uint8_t num_values, unsigned payload_bytes, uint8_t* payload) {
    printf("\n\n------------------------------------------------------------------------------------------------------\n");
    printf("Copy the lines below into the data partition JSON file,\n");
    printf("remove the last comma if it is the last command of the file");
    printf("\n------------------------------------------------------------------------------------------------------\n\n");

    printf("        {\n            \"type\": 3, \"bytes\": [\n");
    // We cannot use current.num_values since some AGC commands overwrite that parameter
    printf("                %3d, %3d, %3d,", current.resid, current.offset, num_values*get_size_from_type(current.type));
    uint8_t byte_cnt = 3;
    for (unsigned i=0; i<payload_bytes-1; i++) {
        if (byte_cnt >= LOG_BYTES_PER_LINE) {
            printf("\n              ");
            byte_cnt = 0;
        }
        printf(" %3d,", payload[i]);
        byte_cnt++;
    }
    printf(" %3d\n", payload[payload_bytes-1]);
    printf("            ]\n        },\n");
    printf("\n------------------------------------------------------------------------------------------------------\n\n");
}

control_ret_t set_struct_val_on_device(cmdspec_t current, int_float *ptr_struct_val, uint8_t log_for_data_partition)
{
    control_ret_t ret = CONTROL_SUCCESS;
    control_resid_t resid = current.resid;
    uint8_t num_values =  current.num_values;

    // update the number of values for the commands with specific endings
    if (!strncmp(current.par_name, "SET_", strlen("SET_")) && \
        (is_same_string_ending(current.par_name, "_CH0_AGC") || is_same_string_ending(current.par_name, "_CH1_AGC"))) {
        num_values= 2;
    }
    unsigned payload_bytes = num_values * current.device_rw_size;
    uint8_t payload[CMD_MAX_BYTES];
    write_payload_byte_array(current, num_values, ptr_struct_val, payload);

    if (log_for_data_partition) {
        print_log_partition_info(current, num_values, payload_bytes, payload);
        return 0;
    }

    if(setup_err != 0)
    {
        printf("control initialization returned error\n");
        return -1;
    }

#if !JSON_ONLY
    int tries = 0;
    do{
        tries += 1;
        ret = control_write_command(resid, (control_cmd_t) (current.offset), (unsigned char *) payload, payload_bytes);
        //in case of error, sleep 15ms and retry again
        //if the error is due to queue being full in the device, the wait and retry should fix it.
        if(ret == CONTROL_ERROR)
        {
            Sleep(15);
        }
    }while((ret != CONTROL_SUCCESS) && (tries < NUM_WRITE_RETRIES));
#endif

    return ret;
}

control_ret_t get_single_struct_val_from_device(cmdspec_t current, int_float *ret_vals, ctrl_flag *done_flag) {
    control_ret_t ret = CONTROL_SUCCESS;
    unsigned char cmd = (unsigned char) current.offset;
    control_resid_t resid = current.resid;
    unsigned payload_bytes = (current.num_values * current.device_rw_size) + 1; //1 extra byte for status
    uint8_t payload[CMD_MAX_BYTES];

#if !JSON_ONLY
    ret = control_read_command(resid, cmd, (unsigned char *) payload, payload_bytes);
    if(ret != CONTROL_SUCCESS)
    {
      return ret;
    }

    if (ret == CONTROL_SUCCESS && payload[0] == CTRL_DONE) {
        read_payload_byte_array(current, current.num_values, &payload[1]/*byte 0 is status*/, ret_vals);
    }
    *done_flag = (ctrl_flag) payload[0];
#endif
    return ret;
}

void get_multiple_struct_val_from_device(cmdspec_t current[], int_float *ret_vals[], unsigned num_cmds, cmdspec_t write_before_retrying[], unsigned *write_val)
{
    int *completed = (int *) calloc(num_cmds, sizeof(int));
    int *sent = (int *) calloc(num_cmds, sizeof(int));
    ctrl_flag *flags = (ctrl_flag*) calloc(num_cmds, sizeof(ctrl_flag));

    int reset_index = 0;

    int running = 1;
    while (running) {
        for (uint32_t i=0; i<num_cmds; i++) {
            if (!completed[i] && ( i == 0 || sent[i-1])) {
                int try_count = 0;
                control_ret_t ret;
                do
                {
                    try_count += 1;
                    ret = get_single_struct_val_from_device(current[i], ret_vals[i], &flags[i]);
                    if(ret != CONTROL_SUCCESS)
                    {
                        if(num_cmds > 1)
                        {
                            printf("Error: resetting start_index not supported when num_simultaneous_cmds is greater than 1\n");
                            host_shutdown(1);
                        }
                        //set coeff index inside the device before retrying read again.
                        int_float val;
                        val.ui = write_val[i];
                        control_ret_t ret_write = set_struct_val_on_device(write_before_retrying[i], &val, 0);
                        if(ret_write != CONTROL_SUCCESS)
                        {
                            printf("Error: cannot write index to the device before retrying coefficients read.\n");
                            host_shutdown(1);
                        }
                        reset_index = 1;
                    }
                }while((ret != CONTROL_SUCCESS) && (try_count < 5));
                if(ret != CONTROL_SUCCESS)
                {
                    printf("error while reading coefficients from device even after %d tries. Exiting\n", try_count);
                    host_shutdown(1);
                }
                sent[i] = (flags[i] == CTRL_DONE || flags[i] == CTRL_WAIT);
                completed[i] = (flags[i] == CTRL_DONE);
            }
            if(reset_index == 1)
            {
                int_float val;
                val.ui = write_val[i] + (AEC_COEFFICIENT_CHUNK_SIZE / sizeof(uint32_t));
                control_ret_t ret_write = set_struct_val_on_device(write_before_retrying[i], &val, 0);
                if(ret_write != CONTROL_SUCCESS)
                {
                    printf("Error: cannot write index to the device.\n");
                    host_shutdown(1);
                }
            }
#if USE_I2C
            Sleep(1);
#endif
        }

        running = 0;
        for (uint32_t i=0; i<num_cmds; i++) {
            running |= !completed[i];
        }
    }

    free(completed);
    free(sent);
    free(flags);
}

control_ret_t get_struct_val_from_device(cmdspec_t current, int_float *ret_vals)
{
    control_ret_t ret = CONTROL_SUCCESS;
    unsigned char cmd = (unsigned char) current.offset;
    control_resid_t resid = current.resid;
    uint8_t num_values = current.num_values;
    if (!strncmp(current.par_name, "GET_", strlen("GET_")) && \
        ( is_same_string_ending(current.par_name, "_CH0_AGC") || is_same_string_ending(current.par_name, "_CH1_AGC"))) {
        num_values = AGC_INPUT_CHANNELS;
    }
    unsigned payload_bytes = (num_values * current.device_rw_size) + 1; //1 extra byte for status
    uint8_t payload[CMD_MAX_BYTES];
    unsigned read_attempts = 0;
    //clock_t start = clock();
#if !JSON_ONLY
    ret = control_read_command(resid, cmd, (unsigned char *) payload, payload_bytes);
    //clock_t end = clock();
    //double time_diff = (((double)end - (double)start)*1000)/(double)CLOCKS_PER_SEC;
    read_attempts += 1;
    while(1)
    {
        if(ret != CONTROL_SUCCESS)
        {
            printf("control_read_command() returned error %d\n",ret);
            break;
        }
        else
        {
            if (read_attempts == 1000) {
                printf("Taking a while... Device status: %d (1:wait, 2:control busy, 3:invalid command)\n", payload[0]);
#ifdef USE_I2C
               printf("NOTE: Control will hang if I2S audio is not playing/recording.\n");
#endif
            }
            if(payload[0] != CTRL_DONE)
            {
                Sleep(1);
                ret = control_read_command(resid, cmd, (unsigned char *) payload, payload_bytes);
                //printf("control_read_command() returned ret = %d payload[0] = %d\n",ret, payload[0]);
                read_attempts += 1;
            }
            else
            {
                read_payload_byte_array(current, num_values, &payload[1]/*byte 0 is status*/, ret_vals);
                break;
            }
        }
    }
    //printf("read_result returning ret=%d after %d read attempts, time_msec = %lf\n",ret, read_attempts, time_diff);
#endif
    return ret;
}

uint64_t convert_energy_to_uint64(float in_val) {
    uint64_t out_val;
    int exp = 0;
    float mant = (float) frexp(in_val, &exp);

    // normalize mantissa and exponent
    uint32_t mant_i = (uint32_t) (mant*UINT_MAX);
    exp -= 32;

    out_val = (uint64_t)mant_i<<32;
    out_val += exp&0xFFFFFFFF;
    return out_val;
}

float convert_uint64_to_energy(uint64_t in_val){
    float out_val;
    uint32_t mant = in_val>>32;
    int32_t exp = in_val&0xffffffff;
    out_val = (float) ldexp(mant, exp);
    return out_val;

}

void calculate_erle(int_float *vals, float *data_out)
{
    float mic_energy = convert_uint64_to_energy(vals[0].i_long);
    float aec_energy = convert_uint64_to_energy(vals[1].i_long);
    float erle = -10 * (float) (log10(aec_energy/mic_energy));
    *data_out = erle;
}

int check_command(unsigned num_args, const char *args, cmdspec_t *cmdspec, int num_commands)
{
    int cmd_num = get_cmdspec_num(cmdspec, num_commands, args);
    if (cmd_num == -1) {
        return -1;
    }
    bool is_set_string =
      strcmp("SET_USB_VENDOR_STRING", cmdspec[cmd_num].par_name) == 0 ||
      strcmp("SET_USB_PRODUCT_STRING", cmdspec[cmd_num].par_name) == 0 ||
      strcmp("SET_SERIAL_NUMBER", cmdspec[cmd_num].par_name) == 0;

    if(cmdspec[cmd_num].rw == READ)
    {
        if(num_args > 1)
        {
            fprintf(stderr, "Error: too many parameters (%d) for a read_command\n", num_args);
            return -2;
        }
    } else if(cmdspec[cmd_num].rw == WRITE && !is_set_string) {
        if((num_args-1) != (cmdspec[cmd_num].num_values)) {
            fprintf(stderr, "Error: command %s expects %d values. user provided %d values\n", args, cmdspec[cmd_num].num_values, num_args-1);
            return -2;
        }
    } else if(cmdspec[cmd_num].rw == WRITE && is_set_string) {
        if(num_args-1 != 1) {
            fprintf(stderr, "Error: command %s expects 1 value. user provided %d values\n", args, num_args-1);
            return -2;
        }
    }
    return cmd_num;
}

int update_read_results(cmdspec_t cmd_spec, int_float *vals, void *data_out_ptr){
    int num_values = cmd_spec.num_values;
    param_type type = cmd_spec.type;
    int err = 0;
    if(cmd_spec.resid == AEC_RESID && (cmd_spec.offset == AEC_CMD_GET_ERLE_CH0 || cmd_spec.offset == AEC_CMD_GET_ERLE_CH1))
    {
        calculate_erle(vals, (float*)data_out_ptr);
        return err;
    }

    switch (type) {
        case TYPE_INT8:
        case TYPE_UINT8:
            for(int i=0; i<num_values; i++)
            {
               ((int8_t*)data_out_ptr)[i] = (int8_t)vals[i].i_long;
            }
            break;
        case TYPE_INT32:
        case TYPE_UINT32:
        case TYPE_TICKS:
            for(int i=0; i<num_values; i++)
            {
               ((int32_t*)data_out_ptr)[i] = (int32_t)vals[i].i_long;
            }
            break;
        case TYPE_INT64:
        case TYPE_UINT64:
            for(int i=0; i<num_values; i++)
            {
               ((int64_t*)data_out_ptr)[i] = vals[i].i_long;
            }
            break;

        case TYPE_FIXED_0_32:
            for(int i=0; i<num_values; i++)
            {
                ((float*)data_out_ptr)[i] = (float)(vals[i].f0_32)/SCALE_Q_32;
            }
            break;
        case TYPE_FIXED_1_31:
            for(int i=0; i<num_values; i++)
            {
                ((float*)data_out_ptr)[i] = (float)(vals[i].f1_31)/SCALE_Q(31);
            }
            break;
        case TYPE_FIXED_7_24:
            for(int i=0; i<num_values; i++)
            {
                ((float*)data_out_ptr)[i] = (float)(vals[i].f7_24)/SCALE_Q(24);
            }
            break;
        case TYPE_FIXED_8_24:
            for(int i=0; i<num_values; i++)
            {
                ((float*)data_out_ptr)[i] = (float)(vals[i].f8_24)/SCALE_Q(24);
            }
            break;
        case TYPE_FIXED_16_16:
            for(int i=0; i<num_values; i++)
            {
                ((float*)data_out_ptr)[i] = (float)(vals[i].f16_16)/SCALE_Q(16);
            }
            break;
        case TYPE_ENERGY:
            for(int i=0; i<num_values; i++)
            {
                ((float*)data_out_ptr)[i] = (float) convert_uint64_to_energy(vals[i].i_long);
            }
            break;
        default:
            printf("Error: invalid type %d\n", type);
            err = -1;
    }
    return err;
}

int do_command(cmdspec_t current, const char ** val_1, void *data_out_ptr, uint8_t log_for_data_partition) {
    unsigned num_values;
    control_ret_t ret = CONTROL_SUCCESS;

    //TODO support --HELP
    num_values = current.num_values;
    int_float vals[CMD_MAX_BYTES] = {0};
    float f = 0;

    bool is_set_string =
      strcmp("SET_USB_VENDOR_STRING", current.par_name) == 0 ||
      strcmp("SET_USB_PRODUCT_STRING", current.par_name) == 0 ||
      strcmp("SET_SERIAL_NUMBER", current.par_name) == 0;

    // remove quotation marks from string argument and check the string size is correct
    char * new_argument_string = NULL;
    if (is_set_string) {
        uint8_t quotation_marks_count = 0;
        new_argument_string = calloc(TEMP_STR_MAX_CHARS, 1);
        uint32_t new_string_idx = 0;
        for (int string_check_idx=0; string_check_idx < (int)strlen(val_1[0]); string_check_idx++) {
            if (val_1[0][string_check_idx] == '"') {
                quotation_marks_count++;
            } else {
                new_argument_string[new_string_idx] = val_1[0][string_check_idx];
                new_string_idx++;
                if (new_string_idx>TEMP_STR_MAX_CHARS) {
                    printf("Error: String in %s is too long and it doesn't fit in the %d-byte buffer, max length is %d\n", current.par_name, TEMP_STR_MAX_CHARS, num_values-1);
                    host_shutdown(1);
                }
            }
        }
        // Reserve last char in array for null-terminator
        if (strlen(new_argument_string) > num_values-1) {
            printf("Error: String in %s is too long, max length is %d, given length is %ld\n", current.par_name, num_values-1, strlen(new_argument_string));
            host_shutdown(1);
        }
    }

    if(current.rw == WRITE) //read the values that the host wants to write
    {
        for(uint32_t i=0; i<num_values; i++)
        {
            switch (current.type) {
                case TYPE_INT8:
                case TYPE_UINT8:
                    if (is_set_string) {
                        vals[i].ui8 = new_argument_string[i];
                    } else {
                        sscanf(val_1[i], "%" SCNu8, &(vals[i].ui8));
                    }
                    break;
                case TYPE_INT32:
                case TYPE_UINT32:
                    sscanf(val_1[i], "%d", &(vals[i].i));
                    break;
                case TYPE_FIXED_0_32:
                    sscanf(val_1[i], "%10f", &(f));
                    vals[i].f0_32 = (uint64_t)(f * SCALE_Q_32);
                    break;
                case TYPE_FIXED_1_31:
                    sscanf(val_1[i], "%10f", &(f));
                    vals[i].f1_31 = (uint32_t)(f * (SCALE_Q(31)));
                    break;
                case TYPE_FIXED_7_24:
                    sscanf(val_1[i], "%8f", &(f));
                    vals[i].f7_24 = (int32_t)(f * (SCALE_Q(24)));
                    break;
                case TYPE_FIXED_8_24:
                    sscanf(val_1[i], "%8f", &(f));
                    vals[i].f8_24 = (uint32_t)(f * (SCALE_Q(24)));
                    break;
                case TYPE_FIXED_16_16:
                    sscanf(val_1[i], "%5f", &(f));
                    vals[i].f16_16 = (uint32_t)(f * (SCALE_Q(16)));
                    break;
                case TYPE_INT64:
                case TYPE_UINT64:
                    sscanf(val_1[i], "%lld", &(vals[i].i_long));
                    break;
                case TYPE_ENERGY:
                    sscanf(val_1[i], "%e", &(f));
                    vals[i].i_long = convert_energy_to_uint64(f);
                    break;
                default:
                  printf("Error: invalid type %d\n", current.type);
                  host_shutdown(1);
            }
        }
    }

    // add the channel index for the commands with specific endings
    if (!strncmp(current.par_name, "SET_", strlen("SET_"))) {
        if (is_same_string_ending(current.par_name, "_CH0_AGC")) {
            vals[num_values].ui = 0;
        } else if (is_same_string_ending(current.par_name, "_CH1_AGC")) {
            vals[num_values].ui = 1;
        }
    }

    if (strcmp("SET_REF_OUT_CH1", current.par_name) == 0) {
        printf("Command for getting reference out on channel 1 is not working in this release\n");
        return CONTROL_ERROR;
    }
    // Get or set result
    if (current.rw == WRITE) {
        if(strcmp("SET_IO_MAP", current.par_name) == 0)
        {
            print_set_io_map((int_float*)vals);
        }
        ret = set_struct_val_on_device(current, vals, log_for_data_partition); //write to device


    } else {
        if (log_for_data_partition) {
            printf("Error: read commands cannot be added to data partition\n");
            return -2;
        }
        if(setup_err != 0)
        {
            printf("control initialization returned error\n");
            return -1;
        }
        ret = get_struct_val_from_device(current, vals); //read from device
    }

    if (!strncmp(current.par_name, "GET_", strlen("GET_")) && is_same_string_ending(current.par_name, "_CH1_AGC")) {
        vals[0] = vals[1];
    }
    //for read commands, copy read values into the output array
    if(!ret && current.rw == READ)
    {
        //copy read results from local output buffer to the output buffer the app sent
        //convert fixed point read results into floating point.
        //convert energy values into dB
        ret = update_read_results(current, vals, data_out_ptr);

    }
    free(new_argument_string);
    return ret;
}

double att_int32_to_double(int32_t x, int x_exp){
    return ldexp((double)x, x_exp);
}

void print_python_fd(FILE *fp, vtb_ch_pair_t * d, size_t length, int d_exp){
    fprintf(fp, "np.asarray([%.12f, ", att_int32_to_double( d[0].ch_a, d_exp));
    for(size_t i=1;i<length;i++){
        fprintf(fp, "%.12f + %.12fj, ", att_int32_to_double( d[i].ch_a, d_exp),
                att_int32_to_double( d[i].ch_b, d_exp));
    }
    fprintf(fp, "%.12f])\n", att_int32_to_double( d[0].ch_b, d_exp));
}

int get_aec_coefficients(cmdspec_t cmdspec_ap[], int num_commands, const char* filename) {
    control_ret_t ret = CONTROL_SUCCESS;

    unsigned x_channel_phases[AEC_MAX_X_CHANNELS];
    unsigned frame_advance = 0;
    unsigned x_channels = 0;
    unsigned y_channels = 0;
    unsigned f_bin_count = 0;

    cmdspec_t get_filter_cmdspec = {0};
    cmdspec_t set_adaption_cmdspec = {0};
    cmdspec_t get_adaption_cmdspec = {0};
    cmdspec_t set_coeff_index_cmdspec = {0};

    int_float *vals = (int_float *) calloc(CMD_MAX_BYTES, sizeof(int_float));

    for (int i=0; i<num_commands; i++) {
        cmdspec_t current = cmdspec_ap[i];
        if (current.resid != AEC_RESID) continue;
        switch (current.offset) {
            case AEC_CMD_GET_FILTER_COEFFICIENTS:
                get_filter_cmdspec = current;
                break;
            case AEC_CMD_SET_ADAPTION_CONFIG:
                set_adaption_cmdspec = current;
                break;
            case AEC_CMD_GET_ADAPTION_CONFIG:
                get_adaption_cmdspec = current;
                break;
            case AEC_CMD_SET_COEFFICIENT_INDEX:
                set_coeff_index_cmdspec = current;
                break;
            case AEC_CMD_GET_FRAME_ADVANCE:
                ret = get_struct_val_from_device(current, vals);
                if (ret != 0) { return 1; }
                frame_advance = vals[0].i;
                printf("frame_advance: %d\n", frame_advance);
                break;
            case AEC_CMD_GET_X_CHANNELS:
                ret = get_struct_val_from_device(current, vals);
                if (ret != 0) { return 1; }
                x_channels = vals[0].i;
                printf("x_channels: %d\n", x_channels);
                break;
            case AEC_CMD_GET_Y_CHANNELS:
                ret = get_struct_val_from_device(current, vals);
                if (ret != 0) { return 1; }
                y_channels = vals[0].i;
                printf("y_channels: %d\n", y_channels);
                break;
            case AEC_CMD_GET_F_BIN_COUNT:
                ret = get_struct_val_from_device(current, vals);
                if (ret != 0) { return 1; }
                f_bin_count = vals[0].i;
                printf("f_bin_count: %d\n", f_bin_count);
                break;
            case AEC_CMD_GET_X_CHANNEL_PHASES:
                ret = get_struct_val_from_device(current, vals);
                if (ret != 0) { return 1; }
                for (uint32_t c=0; c<AEC_MAX_X_CHANNELS; c++) {
                    x_channel_phases[c] = vals[c].ui8;
                }
                break;
        }
    }
    printf("x_channel_phases: ");
    unsigned total_phases = 0;
    unsigned max_phase_count = 0;
    for (uint32_t i=0; i<x_channels; i++) {
        printf("%d ", x_channel_phases[i]);
        total_phases += x_channel_phases[i];
        if (x_channel_phases[i] > max_phase_count) {
            max_phase_count = x_channel_phases[i];
        }
    }
    printf("\n");

    unsigned num_coefficients_per_chunk = AEC_COEFFICIENT_CHUNK_SIZE / sizeof(uint32_t);
    unsigned coeff_size = y_channels * total_phases * (f_bin_count - 1) * 2 + y_channels * total_phases;
    uint32_t *coeff_buffer = (uint32_t *) calloc((num_coefficients_per_chunk + coeff_size), sizeof(uint32_t));

    // Get previous adaption value:
    ret = get_struct_val_from_device(get_adaption_cmdspec, vals);
    int prev_adaption = vals[0].i;

    // Set adaption off
    vals[0].i = AEC_ADAPTION_FORCE_OFF;
    ret = set_struct_val_on_device(set_adaption_cmdspec, vals, 0);
    printf("AEC adaption: off\n");

    // Reset coefficient index
    vals[0].i = 0;
    ret = set_struct_val_on_device(set_coeff_index_cmdspec, vals, 0);
    printf("Reset coefficient index.\n");

    int num_simultaneous_cmds = 1;

    cmdspec_t *cmdspec_arr = (cmdspec_t *) calloc(num_simultaneous_cmds, sizeof(cmdspec_t));
    cmdspec_t *set_index_cmdspec_arr = (cmdspec_t*)calloc(num_simultaneous_cmds, sizeof(cmdspec_t));
    int_float **vals_arr = (int_float **) calloc(num_simultaneous_cmds, sizeof(int_float*));

    unsigned *start_index = (uint32_t*)calloc(num_simultaneous_cmds, sizeof(uint32_t));

    for (int i=0; i<num_simultaneous_cmds; i++) {
        vals_arr[i] = (int_float *) calloc(CMD_MAX_BYTES, sizeof(int_float));
        cmdspec_arr[i] = get_filter_cmdspec;
        set_index_cmdspec_arr[i] = set_coeff_index_cmdspec;
    }

    int print_i = 0;
    int print_period = 5;
    for (uint32_t i=0; i<coeff_size; i++) {
        if (i%(num_simultaneous_cmds * num_coefficients_per_chunk) == 0) {
            for(int index=0; index<num_simultaneous_cmds; index++)
            {
                start_index[index] = i + (index*num_coefficients_per_chunk);
            }
            // Receive coefficients
            get_multiple_struct_val_from_device(cmdspec_arr, vals_arr, num_simultaneous_cmds, set_index_cmdspec_arr, start_index);
            for (int c=0; c<num_simultaneous_cmds; c++) {
                for (uint32_t j=0; j<num_coefficients_per_chunk; j++) {
                    coeff_buffer[i+j+c*num_coefficients_per_chunk] = vals_arr[c][j].i;
                }
            }
            if (print_i <= 0) {
                printf("\r%d / %d", i, coeff_size);
                print_i += print_period;
            }
            fflush(stdout);
            print_i--;
        }
    }
    printf("\r%d / %d\n", coeff_size, coeff_size);

    // Revert adaption
    vals[0].i = prev_adaption;
    ret = set_struct_val_on_device(set_adaption_cmdspec, vals, 0);
    printf("AEC adaption: ");
    switch (prev_adaption) {
        case AEC_ADAPTION_AUTO:
            printf("auto\n");
            break;
        case AEC_ADAPTION_FORCE_ON:
            printf("force on\n");
            break;
        case AEC_ADAPTION_FORCE_OFF:
            printf("force off\n");
            break;
    }

    vtb_ch_pair_t ***H_hat = (vtb_ch_pair_t ***) calloc(y_channels, sizeof(vtb_ch_pair_t **));

    for(uint32_t y_ch=0;y_ch<y_channels;y_ch++) {
        H_hat[y_ch] = (vtb_ch_pair_t **) calloc(total_phases, sizeof(vtb_ch_pair_t *));

        for(unsigned p=0;p<total_phases;p++) {
            H_hat[y_ch][p] = (vtb_ch_pair_t *) calloc((f_bin_count-1), sizeof(vtb_ch_pair_t));
        }
    }

    uint32_t **H_hat_exp = (uint32_t **) calloc(y_channels, sizeof(uint32_t*));

    for(uint32_t y_ch=0;y_ch<y_channels;y_ch++) {
        H_hat_exp[y_ch] = (uint32_t *) calloc(total_phases, sizeof(uint32_t));
    }

    int i=0;
    for(uint32_t y_ch=0;y_ch<y_channels;y_ch++){
        for(unsigned p=0;p<total_phases;p++){
            for (uint32_t j=0; j<f_bin_count-1; j++) {
                for (int re_im=0; re_im<2; re_im++) {
                    if (re_im == 0) {
                        H_hat[y_ch][p][j].ch_a = coeff_buffer[i];
                    } else {
                        H_hat[y_ch][p][j].ch_b = coeff_buffer[i];
                    }
                    i++;
                }
            }
        }
    }
    for(uint32_t y_ch=0;y_ch<y_channels;y_ch++){
        for(unsigned p=0;p<total_phases;p++){
            H_hat_exp[y_ch][p] = coeff_buffer[i];
            i++;
        }
    }

    FILE * fp;
    fp = fopen (filename, "w");

    fprintf(fp, "import numpy as np\n");
    fprintf(fp, "frame_advance = %u\n", frame_advance);
    fprintf(fp, "y_channel_count = %u\n", y_channels);
    fprintf(fp, "x_channel_count = %u\n", x_channels);
    fprintf(fp, "max_phase_count = %u\n", max_phase_count);
    fprintf(fp, "f_bin_count = %u\n", f_bin_count);
    fprintf(fp, "H_hat = np.zeros((y_channel_count, x_channel_count, max_phase_count, f_bin_count), dtype=np.complex128)\n");

    for(uint32_t y_ch=0;y_ch<y_channels;y_ch++){

        unsigned x_ch = 0;
        unsigned x_ch_counter = 0;

        for(unsigned p=0;p<total_phases;p++){

            fprintf(fp, "H_hat[%u][%u][%u] = ", y_ch, x_ch, x_ch_counter);
            print_python_fd(fp, H_hat[y_ch][p], f_bin_count - 1, H_hat_exp[y_ch][p]);

            x_ch_counter++;
            while ((x_ch < x_channels) && (x_ch_counter == x_channel_phases[x_ch])){
                x_ch++;
                x_ch_counter = 0;
            }
        }
    }
    fclose(fp);

    printf("Dumped to %s\n",filename);

    // Free everything

    free(cmdspec_arr);

    for (int c=0; c<num_simultaneous_cmds; c++) {
        free(vals_arr[c]);
    }
    free(vals_arr);

    free(vals);

    for(unsigned y_ch=0;y_ch<y_channels;y_ch++) {
        for(unsigned p=0;p<total_phases;p++) {
            free(H_hat[y_ch][p]);
        }
        free(H_hat[y_ch]);
    }
    free(H_hat);

    for(unsigned y_ch=0;y_ch<y_channels;y_ch++) {
        free(H_hat_exp[y_ch]);
    }
    free(H_hat_exp);

    free(coeff_buffer);

    free(start_index);

    free(set_index_cmdspec_arr);

    return 0;
}

int get_ic_coefficients(cmdspec_t cmdspec_ap[], int num_commands, const char* filename) {
    control_ret_t ret = CONTROL_SUCCESS;

    unsigned phases;
    unsigned proc_frame_bins = 0;

    cmdspec_t get_filter_cmdspec = {0};
    cmdspec_t set_adaption_cmdspec = {0};
    cmdspec_t get_adaption_cmdspec = {0};
    cmdspec_t set_coeff_index_cmdspec = {0};

    int_float *vals = (int_float *) calloc(CMD_MAX_BYTES, sizeof(int_float));

    for (int i=0; i<num_commands; i++) {
        cmdspec_t current = cmdspec_ap[i];
        if (current.resid != IC_RESID) continue;
        switch (current.offset) {
            case IC_CMD_GET_FILTER_COEFFICIENTS:
                get_filter_cmdspec = current;
                break;
            case IC_CMD_SET_ADAPTION_CONFIG:
                set_adaption_cmdspec = current;
                break;
            case IC_CMD_GET_ADAPTION_CONFIG:
                get_adaption_cmdspec = current;
                break;
            case IC_CMD_SET_COEFFICIENT_INDEX:
                set_coeff_index_cmdspec = current;
                break;
            case IC_CMD_GET_PROC_FRAME_BINS:
                ret = get_struct_val_from_device(current, vals);
                if (ret != 0) { return 1; }
                proc_frame_bins = vals[0].i;
                printf("proc_frame_bins: %d\n", proc_frame_bins);
                break;
            case IC_CMD_GET_PHASES:
                ret = get_struct_val_from_device(current, vals);
                if (ret != 0) { return 1; }
                phases = vals[0].ui;
                printf("phases: %d\n", phases);
                break;
        }
    }

    unsigned num_coefficients_per_chunk = AEC_COEFFICIENT_CHUNK_SIZE / sizeof(uint32_t);
    unsigned coeff_size = phases * proc_frame_bins * 2 + phases;
    uint32_t *coeff_buffer = (uint32_t *) calloc((num_coefficients_per_chunk + coeff_size), sizeof(uint32_t));

    // Get previous adaption value:
    ret = get_struct_val_from_device(get_adaption_cmdspec, vals);
    int prev_adaption = vals[0].i;

    // Set adaption off
    vals[0].i = IC_ADAPTION_FORCE_OFF;
    ret = set_struct_val_on_device(set_adaption_cmdspec, vals, 0);
    printf("IC adaption: off\n");

    // Reset coefficient index
    vals[0].i = 0;
    ret = set_struct_val_on_device(set_coeff_index_cmdspec, vals, 0);
    printf("Reset coefficient index.\n");

    int num_simultaneous_cmds = 1;

    cmdspec_t *cmdspec_arr = (cmdspec_t *) calloc(num_simultaneous_cmds, sizeof(cmdspec_t));
    cmdspec_t *set_index_cmdspec_arr = (cmdspec_t*)calloc(num_simultaneous_cmds, sizeof(cmdspec_t));

    int_float **vals_arr = (int_float **) calloc(num_simultaneous_cmds, sizeof(int_float*));

    unsigned *start_index = (uint32_t*)calloc(num_simultaneous_cmds, sizeof(uint32_t));

    for (int i=0; i<num_simultaneous_cmds; i++) {
        vals_arr[i] = (int_float *) calloc(CMD_MAX_BYTES, sizeof(int_float));
        cmdspec_arr[i] = get_filter_cmdspec;
        set_index_cmdspec_arr[i] = set_coeff_index_cmdspec;
    }

    int print_i = 0;
    int print_period = 5;
    for (uint32_t i=0; i<coeff_size; i++) {
        if (i%(num_simultaneous_cmds * num_coefficients_per_chunk) == 0) {
            for(int index=0; index<num_simultaneous_cmds; index++)
            {
                start_index[index] = i + (index*num_coefficients_per_chunk);
            }
            // Receive coefficients
            get_multiple_struct_val_from_device(cmdspec_arr, vals_arr, num_simultaneous_cmds, set_index_cmdspec_arr, start_index);
            for (int c=0; c<num_simultaneous_cmds; c++) {
                for (uint32_t j=0; j<num_coefficients_per_chunk; j++) {
                    coeff_buffer[i+j+c*num_coefficients_per_chunk] = vals_arr[c][j].i;
                }
            }
            if (print_i <= 0) {
                printf("\r%d / %d", i, coeff_size);
                print_i += print_period;
            }
            fflush(stdout);
            print_i--;
        }
    }
    printf("\r%d / %d\n", coeff_size, coeff_size);

    // Revert adaption
    vals[0].i = prev_adaption;
    ret = set_struct_val_on_device(set_adaption_cmdspec, vals, 0);
    printf("IC adaption: ");
    switch (prev_adaption) {
        case IC_ADAPTION_FORCE_ON:
            printf("force on\n");
            break;
        case IC_ADAPTION_FORCE_OFF:
            printf("force off\n");
            break;
    }
    vtb_ch_pair_t **H_hat = (vtb_ch_pair_t **) calloc(phases, sizeof(vtb_ch_pair_t *));

    for(unsigned p=0; p<phases; p++) {
        H_hat[p] = (vtb_ch_pair_t *) calloc(proc_frame_bins, sizeof(vtb_ch_pair_t));
    }

    uint32_t *H_hat_exp = (uint32_t *) calloc(phases, sizeof(uint32_t));

    int i=0;
    for(unsigned p=0; p<phases; p++){
        for (uint32_t j=0; j<proc_frame_bins; j++) {
            for (int re_im=0; re_im<2; re_im++) {
                if (re_im == 0) {
                    H_hat[p][j].ch_a = coeff_buffer[i];
                } else {
                    H_hat[p][j].ch_b = coeff_buffer[i];
                }
                i++;
            }
        }
    }
    for(unsigned p=0; p<phases; p++){
        H_hat_exp[p] = coeff_buffer[i];
        i++;
    }

    FILE * fp;
    fp = fopen (filename, "w");

    fprintf(fp, "import numpy as np\n");
    fprintf(fp, "phases = %u\n", phases);
    fprintf(fp, "proc_frame_bins = %u\n", proc_frame_bins + 1);
    fprintf(fp, "H_hat = np.zeros((phases, proc_frame_bins), dtype=np.complex128)\n");

    for(unsigned p=0; p<phases; p++) {
        fprintf(fp, "H_hat[%u] = ", p);
        print_python_fd(fp, H_hat[p], proc_frame_bins, H_hat_exp[p]);
    }
    fclose(fp);

    printf("Dumped to %s\n",filename);

    // Free everything

    free(cmdspec_arr);

    for (int c=0; c<num_simultaneous_cmds; c++) {
        free(vals_arr[c]);
    }
    free(vals_arr);

    free(vals);

    for(unsigned p=0; p<phases; p++) {
        free(H_hat[p]);
    }
    free(H_hat);

    free(H_hat_exp);

    free(coeff_buffer);

    free(start_index);

    free(set_index_cmdspec_arr);

    return 0;
}



int vfctrl_get_filter_coefficients_human_readable(cmdspec_t *cmd_original){
    cmdspec_t raw_cmd_spec;
    raw_cmd_spec.resid = GPIO_RESID;
    strcpy(raw_cmd_spec.par_name, "GET_FILTER_COEFF_RAW");
    raw_cmd_spec.type = TYPE_INT32;
    raw_cmd_spec.offset = GPIO_CMD_GET_FILTER_COEFF;
    raw_cmd_spec.rw = READ;
    raw_cmd_spec.num_values = NUM_FILTER_COEFFS;
    strcpy(raw_cmd_spec.info, "Stuff");
    raw_cmd_spec.device_rw_size = sizeof(int32_t);
    raw_cmd_spec.app_read_result_size = get_app_read_result_size(raw_cmd_spec.type);

    void* data_out_ptr = calloc(raw_cmd_spec.num_values, raw_cmd_spec.app_read_result_size);
    const unsigned log_for_data_partition = 0; //It's a read so no need to log

    int ret = vfctrl_do_command(&raw_cmd_spec, (const char **)&raw_cmd_spec.par_name, data_out_ptr, log_for_data_partition);

    if(!ret && (raw_cmd_spec.rw == READ)){
        printf("%s: ", cmd_original->par_name);
        double float_vals[MAX_PAYLOAD_BYTES / sizeof(int32_t)];
        for(unsigned i=0; i<raw_cmd_spec.num_values; i++){
            #define FVAL(Q, x) ((double)(x)/(double)(1<<Q))
            double float_val = FVAL(Q_FORMAT_FILTER, ((int32_t*)data_out_ptr)[i]);
            unsigned write_idx = i;
            if(((i%5) == 3) || ((i%5) == 4)){
                float_val = -float_val; //a1 and a2 are negated
                write_idx -= 3;         //reorder from a1,a2,b0,b1,b2
            }
            else{
                write_idx +=2;
            }
            float_vals[write_idx] = float_val;
        }
        for(unsigned i=0; i<raw_cmd_spec.num_values; i++){
            printf("%1.8f ", float_vals[i]);
        }
        printf("\n");
    }
    free(data_out_ptr);
    return ret;
}

int vfctrl_set_filter_coefficients_human_readable(cmdspec_t *cmd_original, const char **command_plus_values, unsigned log_for_data_partition){
    cmdspec_t raw_cmd_spec;
    raw_cmd_spec.resid = GPIO_RESID;
    strcpy(raw_cmd_spec.par_name, "SET_FILTER_COEFF_RAW");
    raw_cmd_spec.type = TYPE_INT32;
    raw_cmd_spec.offset = GPIO_CMD_SET_FILTER_COEFF;
    raw_cmd_spec.rw = WRITE;
    raw_cmd_spec.num_values = 10;
    strcpy(raw_cmd_spec.info, "Stuff");
    raw_cmd_spec.device_rw_size = sizeof(int32_t);
    raw_cmd_spec.app_read_result_size = 0;

    // array of arrays for the int params for ongoing command
    char raw_vals_str[10+1][32] = {{0}};
    strcpy(raw_vals_str[0], raw_cmd_spec.par_name);

    //Grab float values from command and convert to int
    for(unsigned i=0; i<cmd_original->num_values;i++){
        const char * param_str_ptr = command_plus_values[i+1]; //+1 because it's argv
        double set_val;
        sscanf(param_str_ptr, "%le", &set_val);
        unsigned write_idx = i;
        if(((i%5) == 0) || ((i%5) == 1)){
            set_val = -set_val; //a1 and a2 are negated
            write_idx += 3;    //reorder to b0,b1,b2,a1,a2
        }
        else{
            write_idx -= 2;
        }
        #define QVAL(Q, f) (int)((signed long long)((f) * ((unsigned long long)1 << (Q+20)) + (1<<19)) >> 20)
        int32_t q_val = QVAL(Q_FORMAT_FILTER, set_val);
        char number_str[32];
        sprintf(number_str, "%d", q_val);
        strcpy(raw_vals_str[write_idx+1], number_str);
        // printf("%lf@@@%s\n", set_val, raw_vals_str[i+1]);
    }

    //Now build an array of pointers pointing into the str arrays so it looks like *argv[]
    char *raw_vals_str_ptrs[10+1];
    for(int i=1;i<11;i++) raw_vals_str_ptrs[i] = raw_vals_str[i];

    int ret = vfctrl_do_command(&raw_cmd_spec, (const char ** )raw_vals_str_ptrs, NULL, log_for_data_partition);
    return ret;
}

char* print_help(char* bin_name, cmdspec_t *cmdspec_ap, unsigned num_commands, unsigned full) {
    int len = 0;
    printf("\nUsage: %s COMMAND [VALUES ...]", bin_name);
    printf("\n");

    printf("Use -h or --help to list possible commands.\n");
    printf("Use -v or --vendor-id to set the vendor ID. Default value is 0x20B1\n");
    printf("Use -p or --product-id to set the product ID. Default value is 0x0014\n");
    printf("Use -d or --dump-params to read all the available parameters.\n");
    printf("Use -l or --log-data-partition to generate the json item to use in the flash data-partition\n");

    if (!full) {
        return "";
    }

    printf("\n\n");
    printf("In case of fixed-point values, enter an unsigned rational number in the range indicated by the format type\n\n");
    printf("%-30s    %-12s    %-5s     %-10s    %-50s\n",
            "Command", "Type", "R/W", "Num Values", "Info");
    printf("\n");

    for (uint32_t i=0; i<num_commands; i++) {
        cmdspec_t cmd = cmdspec_ap[i];

        char *type;
        switch (cmd.type) {
            case TYPE_ENERGY:
                type = "ENERGY"; break;
            case TYPE_FIXED_0_32:
                type = "FIXED_0_32"; break;
            case TYPE_FIXED_1_31:
                type = "FIXED_1_31"; break;
            case TYPE_FIXED_7_24:
                type = "FIXED_7_24"; break;
            case TYPE_FIXED_8_24:
                type = "FIXED_8_24"; break;
            case TYPE_FIXED_16_16:
                type = "FIXED_16_16"; break;
            case TYPE_INT8:
                type = "INT8"; break;
            case TYPE_UINT8:
                type = "UINT8"; break;
            case TYPE_INT32:
                type = "INT32"; break;
            case TYPE_UINT32:
                type = "UINT32"; break;
            case TYPE_INT64:
                type = "INT64"; break;
            case TYPE_UINT64:
                type = "UINT64"; break;
            case TYPE_TICKS:
                type = "TICKS"; break;
            default:
                type = "NA"; break;
        }

        char* rw = "READ";
        if (cmd.rw) rw = "WRITE";
        len += sprintf(cmd_list + len, "%s ", cmd.par_name);
        printf("%-30s    %-12s    %-5s     %-10d    %-50s\n",
                cmd.par_name, type, rw, cmd.num_values, cmd.info);

    }
    return cmd_list;
}

void populate_cmd_table()
{
    if(cmdspec_ap == NULL)
    {
        cmdspec_t cmdspec_ap_local[] = {
            cmd_con(AP_CONTROL_RESID, "GET_VERSION", TYPE_UINT32, AP_CONTROL_CMD_GET_VERSION, READ, 1, "CONTROL: get version"),
            cmd_con(AP_CONTROL_RESID, "GET_STATUS", TYPE_UINT8, AP_CONTROL_CMD_GET_STATUS, READ, 1, "CONTROL: get status"),
            cmd_con(AP_CONTROL_RESID, "GET_DELAY_SAMPLES", TYPE_UINT32, AP_CONTROL_CMD_GET_DELAY_SAMPLES, READ, 1, "CONTROL: get configurable delay in samples"),
            cmd_con(AP_CONTROL_RESID, "GET_BLD_MSG", TYPE_UINT8, AP_CONTROL_CMD_GET_BLD_MSG, READ, BLD_MSG_LEN, "CONTROL: get build message"),
            cmd_con(AP_CONTROL_RESID, "GET_BLD_HOST", TYPE_UINT8, AP_CONTROL_CMD_GET_BLD_HOST, READ, BLD_HOST_LEN, "CONTROL: get build host"),
            cmd_con(AP_CONTROL_RESID, "GET_BLD_REPO_HASH", TYPE_UINT8, AP_CONTROL_CMD_GET_BLD_REPO_HASH, READ, BLD_REPO_HASH_LEN, "CONTROL: get repo hash"),
            cmd_con(AP_CONTROL_RESID, "GET_BLD_XGIT_VIEW", TYPE_UINT8, AP_CONTROL_CMD_GET_BLD_XGIT_VIEW, READ, BLD_XGIT_VIEW_LEN, "CONTROL: get xgit view"),
            cmd_con(AP_CONTROL_RESID, "GET_BLD_XGIT_HASH", TYPE_UINT8, AP_CONTROL_CMD_GET_BLD_XGIT_HASH, READ, BLD_XGIT_HASH_LEN, "CONTROL: get xgit hash"),
            cmd_con(AP_CONTROL_RESID, "GET_BLD_MODIFIED", TYPE_UINT8, AP_CONTROL_CMD_GET_BLD_MODIFIED, READ, BLD_MODIFIED_LEN, "CONTROL: get build modified from given view/hash"),
#if !USE_I2C
            cmd_con(GPIO_RESID, "GET_I2C", TYPE_UINT8, GPIO_CMD_GET_I2C, READ, I2C_MAX_CMD_SIZE, "GPIO: Read from an I2C device connected to the xvf device"),
            cmd_con(GPIO_RESID, "GET_I2C_WITH_REG", TYPE_UINT8, GPIO_CMD_GET_I2C_WITH_REG, READ, I2C_MAX_CMD_SIZE, "GPIO: Read from the register of an I2C device connected to the xvf device"),
            cmd_con(GPIO_RESID, "GET_I2C_READ_HEADER", TYPE_UINT8, GPIO_CMD_GET_I2C_READ_HEADER, READ, 3, "GPIO: Get the address, register address, and count of next I2C read"),
            cmd_con(GPIO_RESID, "SET_I2C", TYPE_UINT8, GPIO_CMD_SET_I2C, WRITE, I2C_MAX_CMD_SIZE, "GPIO: Write to an I2C device connected to the xvf device"),
            cmd_con(GPIO_RESID, "SET_I2C_WITH_REG", TYPE_UINT8, GPIO_CMD_SET_I2C_WITH_REG, WRITE, I2C_MAX_CMD_SIZE, "GPIO: Write to the register of an I2C device connected to the xvf device"),
            cmd_con(GPIO_RESID, "SET_I2C_READ_HEADER", TYPE_UINT8, GPIO_CMD_SET_I2C_READ_HEADER, WRITE, 3, "GPIO: Set address, register address, and count of next I2C read"),

#endif // !USE_I2C
            cmd_con(GPIO_RESID, "GET_SPI", TYPE_UINT8, GPIO_CMD_GET_SPI, READ, SPI_READ_BUF_SIZE, "GPIO: Gets the contents of the SPI read buffer"),
            cmd_con(GPIO_RESID, "GET_SPI_READ_HEADER", TYPE_UINT8, GPIO_CMD_GET_SPI_READ_HEADER, READ, 2, "GPIO: Get the address and count of next SPI read"),
            cmd_con(GPIO_RESID, "SET_SPI_PUSH", TYPE_UINT8, GPIO_CMD_SET_SPI_PUSH, WRITE, SPI_MAX_CMD_SIZE, "GPIO: Push SPI command data onto the execution queue"),
            cmd_con(GPIO_RESID, "SET_SPI_PUSH_AND_EXEC", TYPE_UINT8, GPIO_CMD_SET_SPI_PUSH_AND_EXEC, WRITE, SPI_MAX_CMD_SIZE, "GPIO: Push SPI command data and execute the command from the stack"),
            cmd_con(GPIO_RESID, "SET_SPI_READ_HEADER", TYPE_UINT8, GPIO_CMD_SET_SPI_READ_HEADER, WRITE, 2, "GPIO: Set address and count of next SPI read"),

            cmd_con(GPIO_RESID, "GET_GPI_PORT", TYPE_UINT32, GPIO_CMD_GET_GPI_PORT, READ, 1, "GPIO: Read current state of the selected GPIO port"),
            cmd_con(GPIO_RESID, "GET_GPI_PIN", TYPE_UINT32, GPIO_CMD_GET_GPI_PIN, READ, 1, "GPIO: Read current state of the selected GPIO pin"),
            cmd_con(GPIO_RESID, "GET_GPI_INT_PENDING_PIN", TYPE_UINT32, GPIO_CMD_GET_GPI_INT_PENDING_PIN, READ, 1, "GPIO: Read whether interrupt was triggered for selected pin"),
            cmd_con(GPIO_RESID, "GET_GPI_INT_PENDING_PORT", TYPE_UINT32, GPIO_CMD_GET_GPI_INT_PENDING_PORT, READ, 1, "GPIO: Read whether interrupt was triggered for all pins on selected port"),
            cmd_con(GPIO_RESID, "GET_KWD_HID_EVENT_CNT", TYPE_UINT32, GPIO_CMD_GET_KWD_HID_EVENT_CNT, READ, 1, "GPIO: read number of KWD HID events detected"),
            cmd_con(GPIO_RESID, "SET_KWD_HID_EVENT_CNT", TYPE_UINT32, GPIO_CMD_SET_KWD_HID_EVENT_CNT, WRITE, 1, "GPIO: write number of KWD HID events detected"),
            cmd_con(GPIO_RESID, "SET_GPO_PORT", TYPE_UINT32, GPIO_CMD_SET_GPO_PORT, WRITE, 2, "GPIO: Write to all pins of a GPIO port"),
            cmd_con(GPIO_RESID, "SET_GPO_PIN", TYPE_UINT8, GPIO_CMD_SET_GPO_PIN, WRITE, 3, "GPIO: Write to a specific GPIO pin"),
            cmd_con(GPIO_RESID, "SET_GPO_PIN_ACTIVE_LEVEL", TYPE_UINT8, GPIO_CMD_SET_GPO_PIN_ACTIVE_LEVEL, WRITE, 3, "GPIO: Set the active level for a specific GPO pin. 0: active low, 1: active high"),
            cmd_con(GPIO_RESID, "SET_GPI_PIN_ACTIVE_LEVEL", TYPE_UINT8, GPIO_CMD_SET_GPI_PIN_ACTIVE_LEVEL, WRITE, 3, "GPIO: Set the active level for a specific GPI pin. 0: active low, 1: active high"),
            cmd_con(GPIO_RESID, "SET_GPI_INT_CONFIG", TYPE_UINT8, GPIO_CMD_SET_GPI_INT_CONFIG, WRITE, 3, "GPIO: Sets the interrupt config for a specific pin"),
            cmd_con(GPIO_RESID, "SET_GPI_READ_HEADER", TYPE_UINT8, GPIO_CMD_SET_GPI_READ_HEADER, WRITE, 2, "GPIO: Sets the selected port and pin for the next GPIO read"),
            cmd_con(GPIO_RESID, "GET_GPI_READ_HEADER", TYPE_UINT8, GPIO_CMD_GET_GPI_READ_HEADER, READ, 2, "GPIO: Gets the currently selected port and pin"),
            cmd_con(GPIO_RESID, "SET_GPO_PWM_DUTY", TYPE_UINT8, GPIO_CMD_SET_GPO_PWM_DUTY, WRITE, 3, "GPIO: Set the pwm duty for a specific pin. Value given as an integer percentage"),
            cmd_con(GPIO_RESID, "SET_GPO_FLASHING", TYPE_UINT32, GPIO_CMD_SET_GPO_FLASHING, WRITE, 3, "GPIO: Set the serial flash mask for a specific pin. Each bit in the mask describes the GPO state for 100ms intervals"),

            cmd_con(GPIO_RESID, "GET_KWD_BOOT_STATUS", TYPE_UINT8, GPIO_CMD_GET_KWD_BOOT_STATUS, READ, 1, "GPIO: Gets boot status for keyword detectors"),
            cmd_con(GPIO_RESID, "GET_RUN_STATUS", TYPE_UINT8, GPIO_CMD_GET_RUN_STATUS, READ, 1, "GPIO: Gets run status for the device"),
            cmd_con(GPIO_RESID, "GET_IO_MAP_AND_SHIFT", TYPE_UINT8, GPIO_CMD_GET_IO_MAP_AND_SHIFT, READ, NUM_IO_MAP_OUTPUTS*sizeof(io_map_and_output_shift_t), "Get IO map and output shift values for the device"),
            cmd_con(GPIO_RESID, "SET_IO_MAP", TYPE_UINT8, GPIO_CMD_SET_IO_MAP, WRITE, 2, "Set IO map for the device. arg1: dest(output_io_map_t), arg2: source(input_io_map_t)"),

            cmd_con(GPIO_RESID, "SET_FILTER_INDEX", TYPE_UINT8, GPIO_CMD_SET_FILTER_INDEX, WRITE, 1, "Set filter index. Selects which filter block will be read from/written to arg1: dest(filter_block_map_t)"),
            cmd_con(GPIO_RESID, "SET_FILTER_BYPASS", TYPE_UINT8, GPIO_CMD_SET_FILTER_BYPASS, WRITE, 1, "Set filter bypass state. arg1: 0 - filter enabled, 1 - bypassed"),
            cmd_con(GPIO_RESID, "SET_FILTER_COEFF", TYPE_ENERGY, GPIO_CMD_SET_FILTER_COEFF, WRITE, NUM_FILTER_COEFFS, "Set biquad coeffs for a selected filter using floating point. arg1..10: 5x2 float coeffs in forward order (a1,a2,b0,b1,b2) where a0 always is 1.0"),
            cmd_con(GPIO_RESID, "SET_FILTER_COEFF_RAW", TYPE_INT32, GPIO_CMD_SET_FILTER_COEFF, WRITE, NUM_FILTER_COEFFS, "Set raw biquad coeffs for a selected filters. arg1..10: 2 sets of coeffs in forward order (b0,b1,b2,-a1,-a2) signed Q28 format"),

            cmd_con(GPIO_RESID, "GET_FILTER_INDEX", TYPE_UINT8, GPIO_CMD_GET_FILTER_INDEX, READ, 1, "Get filter index. Selects which filter block will be read from/written to"),
            cmd_con(GPIO_RESID, "GET_FILTER_BYPASS", TYPE_UINT8, GPIO_CMD_GET_FILTER_BYPASS, READ, 1, "Get filter bypass state. 0 - filter enabled, 1 - bypassed"),
            cmd_con(GPIO_RESID, "GET_FILTER_COEFF", TYPE_ENERGY, GPIO_CMD_GET_FILTER_COEFF, READ, NUM_FILTER_COEFFS, "Get biquad coeffs for a selected filter using floating point. arg1..10: 5x2 float coeffs in forward order (a1,a2,b0,b1,b2) where a0 always is 1.0"),
            cmd_con(GPIO_RESID, "GET_FILTER_COEFF_RAW", TYPE_INT32, GPIO_CMD_GET_FILTER_COEFF, READ, NUM_FILTER_COEFFS, "Get raw biquad coeffs for a selected filters. arg1..10: 2 sets of coeffs in forward order (b0,b1,b2,-a1,-a2) signed Q28 format"),

            cmd_con(GPIO_RESID, "SET_OUTPUT_SHIFT", TYPE_INT32, GPIO_CMD_SET_OUTPUT_SHIFT, WRITE, 2, "For a selected output(output_io_map_t), set the no. of bits the output samples will be shifted by. Postive shift value indicates left shift, negative indicates right shift"),
            cmd_con(GPIO_RESID, "GET_MAX_UBM_CYCLES", TYPE_UINT32, GPIO_CMD_GET_MAX_UBM_CYCLES, READ, 1, "Get maximum no. of cycles taken by the user buffer management function"),
            cmd_con(GPIO_RESID, "RESET_MAX_UBM_CYCLES", TYPE_UINT8, GPIO_CMD_RESET_MAX_UBM_CYCLES, WRITE, 1, "reset the max user buffer management cycles count"),
            cmd_con(GPIO_RESID, "GET_I2S_RATE", TYPE_UINT32, GPIO_CMD_GET_I2S_RATE, READ, 1, "Get I2S rate"),
            cmd_con(GPIO_RESID, "SET_I2S_RATE", TYPE_UINT32, GPIO_CMD_SET_I2S_RATE, WRITE, 1, "Set I2S rate. This command is only run from the flash. Run it only with -l option to generate what goes in the flash data-partition."),
            cmd_con(GPIO_RESID, "SET_I2S_START_STATUS", TYPE_UINT8, GPIO_CMD_SET_I2S_START_STATUS, WRITE, 1, "Start I2S. This command is only run from the flash. Run it only with -l option to generate the json item to use in the flash data-partition"),
            cmd_con(GPIO_RESID, "GET_I2S_START_STATUS", TYPE_UINT8, GPIO_CMD_GET_I2S_START_STATUS, READ, 1, "Get I2S start status"),
            cmd_con(GPIO_RESID, "GET_USB_VENDOR_ID", TYPE_UINT32, GPIO_CMD_GET_USB_VENDOR_ID, READ, 1, "Get USB Vendor ID"),
            cmd_con(GPIO_RESID, "GET_USB_PRODUCT_ID", TYPE_UINT32, GPIO_CMD_GET_USB_PRODUCT_ID, READ, 1, "Get USB Product ID"),
            cmd_con(GPIO_RESID, "GET_USB_BCD_DEVICE", TYPE_UINT32, GPIO_CMD_GET_USB_BCD_DEVICE, READ, 1, "Get USB Device Release Number (bcdDevice)"),
            cmd_con(GPIO_RESID, "GET_USB_VENDOR_STRING", TYPE_UINT8, GPIO_CMD_GET_USB_VENDOR_STRING, READ, USB_STR_MAX_BYTES, "Get USB Vendor string"),
            cmd_con(GPIO_RESID, "GET_USB_PRODUCT_STRING", TYPE_UINT8, GPIO_CMD_GET_USB_PRODUCT_STRING, READ, USB_STR_MAX_BYTES, "Get USB Product string"),
            cmd_con(GPIO_RESID, "GET_SERIAL_NUMBER", TYPE_UINT8, GPIO_CMD_GET_SERIAL_NUMBER, READ, USB_STR_MAX_BYTES, "Read serial number from USB descriptor (normally initialised from flash)."),
            cmd_con(GPIO_RESID, "GET_HARDWARE_BUILD", TYPE_UINT32, GPIO_CMD_GET_HARDWARE_BUILD, READ, 1, "Get the build number from the hardware build section of the flash data partition."),

            cmd_con(GPIO_RESID, "SET_USB_VENDOR_ID", TYPE_UINT32, GPIO_CMD_SET_USB_VENDOR_ID, WRITE, 1, "Set USB Vendor ID. This command is only run from the flash. Run it only with -l option to generate the json item to use in the flash data-partition"),
            cmd_con(GPIO_RESID, "SET_USB_PRODUCT_ID", TYPE_UINT32, GPIO_CMD_SET_USB_PRODUCT_ID, WRITE, 1, "Set USB Product ID. This command is only run from the flash. Run it only with -l option to generate the json item to use in the flash data-partition"),
            cmd_con(GPIO_RESID, "SET_USB_VENDOR_STRING", TYPE_UINT8, GPIO_CMD_SET_USB_VENDOR_STRING, WRITE, USB_STR_MAX_BYTES, "Set USB Vendor string. This command is only run from the flash. Run it only with -l option to generate the json item to use in the flash data-partition"),
            cmd_con(GPIO_RESID, "SET_USB_PRODUCT_STRING", TYPE_UINT8, GPIO_CMD_SET_USB_PRODUCT_STRING, WRITE, USB_STR_MAX_BYTES, "Set USB Product string. This command is only run from the flash. Run it only with -l option to generate the json item to use in the flash data-partition"),
            cmd_con(GPIO_RESID, "SET_SERIAL_NUMBER", TYPE_UINT8, GPIO_CMD_SET_SERIAL_NUMBER, WRITE, USB_STR_MAX_BYTES, "Program serial number to flash"),
            cmd_con(GPIO_RESID, "SET_USB_SERIAL_NUMBER", TYPE_UINT8, GPIO_CMD_SET_USB_SERIAL_NUMBER, WRITE, 1, "Load serial number from flash and initialise USB device descriptor with it. Will not work after boot since descriptor is populated only once, with USB start."),

            cmd_con(GPIO_RESID, "SET_USB_BCD_DEVICE", TYPE_UINT32, GPIO_CMD_SET_USB_BCD_DEVICE, WRITE, 1, "Set USB Device Release Number (bcdDevice)"),
#if !USE_I2C
            cmd_con(GPIO_RESID, "SET_USB_START_STATUS", TYPE_UINT8, GPIO_CMD_SET_USB_START_STATUS, WRITE, 1, "Start USB. This command is only run from the flash. Run it only with -l option to generate the json item to use in the flash data-partition"),
            cmd_con(GPIO_RESID, "GET_USB_START_STATUS", TYPE_UINT8, GPIO_CMD_GET_USB_START_STATUS, READ, 1, "Get USB start status"),
#endif // !USE_I2C
            cmd_con(GPIO_RESID, "GET_USB_TO_DEVICE_RATE", TYPE_UINT32, GPIO_CMD_GET_USB_TO_DEVICE_RATE, READ, 1, "Get USB to device rate"),
            cmd_con(GPIO_RESID, "GET_DEVICE_TO_USB_RATE", TYPE_UINT32, GPIO_CMD_GET_DEVICE_TO_USB_RATE, READ, 1, "Get device to USB rate"),
            cmd_con(GPIO_RESID, "GET_USB_TO_DEVICE_BIT_RES", TYPE_UINT32, GPIO_CMD_GET_USB_TO_DEVICE_BIT_RES, READ, 1, "Get USB to device bit resolution"),
            cmd_con(GPIO_RESID, "GET_DEVICE_TO_USB_BIT_RES", TYPE_UINT32, GPIO_CMD_GET_DEVICE_TO_USB_BIT_RES, READ, 1, "Get device to USB bit resolution"),
            cmd_con(GPIO_RESID, "SET_USB_TO_DEVICE_RATE", TYPE_UINT32, GPIO_CMD_SET_USB_TO_DEVICE_RATE, WRITE, 1, "Set USB to device rate"),
            cmd_con(GPIO_RESID, "SET_DEVICE_TO_USB_RATE", TYPE_UINT32, GPIO_CMD_SET_DEVICE_TO_USB_RATE, WRITE, 1, "Set device to USB rate"),
            cmd_con(GPIO_RESID, "SET_USB_TO_DEVICE_BIT_RES", TYPE_UINT32, GPIO_CMD_SET_USB_TO_DEVICE_BIT_RES, WRITE, 1, "Set USB to device bit resolution"),
            cmd_con(GPIO_RESID, "SET_DEVICE_TO_USB_BIT_RES", TYPE_UINT32, GPIO_CMD_SET_DEVICE_TO_USB_BIT_RES, WRITE, 1, "Set device to USB bit resolution"),

            cmd_con(GPIO_RESID, "GET_MCLK_IN_TO_PDM_CLK_DIVIDER", TYPE_UINT8, GPIO_CMD_GET_MCLK_IN_TO_PDM_CLK_DIVIDER, READ, 1, "Get XCore divider from input master clock to 6.144MHz DDR PDM microphone clock"),
            cmd_con(GPIO_RESID, "GET_SYS_CLK_TO_MCLK_OUT_DIVIDER", TYPE_UINT8, GPIO_CMD_GET_SYS_CLK_TO_MCLK_OUT_DIVIDER, READ, 1, "Get XCore divider from system clock to output master clock"),
            cmd_con(GPIO_RESID, "SET_MCLK_IN_TO_PDM_CLK_DIVIDER", TYPE_UINT8, GPIO_CMD_SET_MCLK_IN_TO_PDM_CLK_DIVIDER, WRITE, 1, "Set XCore divider from input master clock to 6.144MHz DDR PDM microphone clock (when master clock is slaved). Run it only with -l option to generate the json item to use in the flash data-partition"),
            cmd_con(GPIO_RESID, "SET_SYS_CLK_TO_MCLK_OUT_DIVIDER", TYPE_UINT8, GPIO_CMD_SET_SYS_CLK_TO_MCLK_OUT_DIVIDER, WRITE, 1, "Set XCore divider from system clock to output master clock (where master clock output is used). Run it only with -l option to generate the json item to use in the flash data-partition"),
            cmd_con(GPIO_RESID, "GET_MIC_START_STATUS", TYPE_UINT8, GPIO_CMD_GET_MIC_START_STATUS, READ, 1, "Get microphone client start status."),
            cmd_con(GPIO_RESID, "SET_MIC_START_STATUS", TYPE_UINT8, GPIO_CMD_SET_MIC_START_STATUS, WRITE, 1, "Start microphone client (audio frontend).  This command is only run from the flash. Run it only with -l option to generate the json item to use in the flash data-partition"),
            cmd_con(GPIO_RESID, "SET_MONITOR_STATE_USING_GPO_ENABLED", TYPE_UINT8, GPIO_CMD_SET_MONITOR_STATE_USING_GPO_ENABLED, WRITE, 1, "enable monitoring of state on GPO. This command is only run from the flash. Run it only with -l option to generate the json item to use in the flash data-partition"),
            cmd_con(GPIO_RESID, "SET_KWD_INTERRUPT_PIN", TYPE_UINT8, GPIO_CMD_SET_KWD_INTERRUPT_PIN, WRITE, 1, "set gpi pin index to receive kwd interrupt on"),
            cmd_con(GPIO_RESID, "GET_KWD_INTERRUPT_PIN", TYPE_UINT8, GPIO_CMD_GET_KWD_INTERRUPT_PIN, READ, 1, "get gpi pin index to receive kwd interrupt on"),

            cmd_con(AEC_RESID,      "GET_BYPASS_AEC", TYPE_INT32, AEC_CMD_GET_BYPASS, READ, 1, "AEC: get bypass"),
            cmd_con(AEC_RESID,      "GET_X_ENERGY_DELTA_AEC", TYPE_ENERGY, AEC_CMD_GET_X_ENERGY_DELTA, READ, 1, "AEC: get X energy delta"),
            cmd_con(AEC_RESID,      "GET_X_ENERGY_GAMMA_LOG2_AEC", TYPE_INT32, AEC_CMD_GET_X_ENERGY_GAMMA_LOG2, READ, 1, "AEC: get X energy gamma log2"),
            cmd_con(AEC_RESID,      "GET_FORCED_MU_VALUE_AEC", TYPE_FIXED_1_31, AEC_CMD_GET_FORCED_MU_VALUE, READ, 1, "AEC: get forced mu value"),
            cmd_con(AEC_RESID,      "GET_ADAPTATION_CONFIG_AEC", TYPE_INT32, AEC_CMD_GET_ADAPTION_CONFIG, READ, 1, "AEC: get adaptation config"),
            cmd_con(AEC_RESID,      "GET_MU_SCALAR_AEC", TYPE_FIXED_8_24, AEC_CMD_GET_MU_SCALAR, READ, 1, "AEC: get mu_scalar"),
            cmd_con(AEC_RESID,      "GET_MU_LIMITS_AEC", TYPE_FIXED_1_31, AEC_CMD_GET_MU_LIMITS, READ, 2, "AEC: get mu_high and mu_low"),
            cmd_con(AEC_RESID,      "GET_SIGMA_ALPHAS_AEC", TYPE_UINT32, AEC_CMD_GET_SIGMA_ALPHAS, READ, 3, "AEC: get sigma alphas"),
            cmd_con(AEC_RESID,      "GET_ERLE_CH0_AEC", TYPE_ENERGY, AEC_CMD_GET_ERLE_CH0, READ, 2, "AEC: get channel 0 ERLE"),
            cmd_con(AEC_RESID,      "GET_ERLE_CH1_AEC", TYPE_ENERGY, AEC_CMD_GET_ERLE_CH1, READ, 2, "AEC: get channel 1 ERLE"),
            cmd_con(AEC_RESID,      "GET_FRAME_ADVANCE_AEC", TYPE_UINT32, AEC_CMD_GET_FRAME_ADVANCE, READ, 1, "AEC: get frame advance"),
            cmd_con(AEC_RESID,      "GET_Y_CHANNELS_AEC", TYPE_UINT32, AEC_CMD_GET_Y_CHANNELS, READ, 1, "AEC: get y channels"),
            cmd_con(AEC_RESID,      "GET_X_CHANNELS_AEC", TYPE_UINT32, AEC_CMD_GET_X_CHANNELS, READ, 1, "AEC: get x channels"),
            cmd_con(AEC_RESID,      "GET_X_CHANNEL_PHASES_AEC", TYPE_UINT8, AEC_CMD_GET_X_CHANNEL_PHASES, READ, AEC_MAX_X_CHANNELS, "AEC: get x channel phases"),
            cmd_con(AEC_RESID,      "GET_F_BIN_COUNT_AEC", TYPE_UINT32, AEC_CMD_GET_F_BIN_COUNT, READ, 1, "AEC: get f bin count"),
            cmd_con(AEC_RESID,      "GET_FILTER_COEFFICIENTS_AEC", TYPE_INT32, AEC_CMD_GET_FILTER_COEFFICIENTS, READ, AEC_COEFFICIENT_CHUNK_SIZE/sizeof(uint32_t), "AEC: get filter coefficients"),
            cmd_con(AEC_RESID,      "GET_COEFF_INDEX_AEC", TYPE_UINT32, AEC_CMD_GET_COEFFICIENT_INDEX, READ, 1, "AEC: get coefficient index"),
            cmd_con(IC_RESID,      "GET_CH1_BEAMFORM_ENABLE", TYPE_UINT8, IC_CMD_GET_CH1_BEAMFORM_ENABLE, READ, 1, "get if beamforming is enabled on channel1. default:enable"),

            cmd_con(AP_STAGE_A_RESID, "GET_DELAY_ESTIMATOR_ENABLED", TYPE_UINT32, AP_STAGE_A_CMD_GET_DELAY_ESTIMATOR_ENABLED, READ, 1, "CONTROL: enable/disable delay estimation"),
            cmd_con(AP_STAGE_A_RESID, "GET_DELAY_ESTIMATE", TYPE_INT32, AP_STAGE_A_CMD_GET_DELAY_ESTIMATE, READ, 1, "CONTROL: get delay estimate"),
            cmd_con(AP_STAGE_A_RESID, "GET_DELAY_DIRECTION", TYPE_UINT32, AP_STAGE_A_CMD_GET_DELAY_DIRECTION, READ, 1, "CONTROL: get configurable delay direction: 0: delay references, 1: delay mics"),
            cmd_con(AP_STAGE_A_RESID, "GET_MIC_SHIFT_SATURATE", TYPE_UINT32, AP_STAGE_A_CMD_GET_MIC_SHIFT_SATURATE, READ, 2, "CONTROL: get the shift value and saturation (1=enable) to be applied to the input mic samples"),
            cmd_con(AP_STAGE_A_RESID, "GET_ALT_ARCH_ENABLED", TYPE_UINT32, AP_STAGE_A_CMD_GET_ALT_ARCH_ENABLED, READ, 1, "stage A: Get state of xvf3510 alternate architecture setting: 0: normal, 1: alt-arch"),

            cmd_con(AP_STAGE_A_RESID, "GET_ADEC_ENABLED", TYPE_UINT32, AP_STAGE_A_CMD_GET_ADEC_ENABLED, READ, 1, "stage A: get automatic delay estimator controller enabled: 0: off, 1: on"),
            cmd_con(AP_STAGE_A_RESID, "GET_ADEC_MODE", TYPE_UINT32, AP_STAGE_A_CMD_GET_ADEC_MODE, READ, 1, "stage A: get automatic delay estimator controller mode: 0: normal AEC mode, 1: delay estimation mode"),
            cmd_con(AP_STAGE_A_RESID, "GET_ADEC_TIME_SINCE_RESET", TYPE_UINT32, AP_STAGE_A_CMD_GET_ADEC_TIME_SINCE_RESET, READ, 1, "stage A: get time in milliseconds since last automatic delay change by ADEC"),
            cmd_con(AP_STAGE_A_RESID, "GET_AGM", TYPE_FIXED_7_24, AP_STAGE_A_CMD_GET_AGM, READ, 1, "stage A: get AEC Goodness Metric estimate (0.0 - 1.0"),
            cmd_con(AP_STAGE_A_RESID, "GET_ERLE_BAD_BITS", TYPE_FIXED_7_24, AP_STAGE_A_CMD_GET_ERLE_BAD_BITS, READ, 1, "stage A: get ERLE bad threshold in bits (log2)"),
            cmd_con(AP_STAGE_A_RESID, "GET_ERLE_GOOD_BITS", TYPE_FIXED_7_24, AP_STAGE_A_CMD_GET_ERLE_GOOD_BITS, READ, 1, "stage A: get ERLE good threshold in bits (log2)"),
            cmd_con(AP_STAGE_A_RESID, "GET_PEAK_PHASE_ENERGY_TREND_GAIN", TYPE_FIXED_7_24, AP_STAGE_A_CMD_GET_PEAK_PHASE_ENERGY_TREND_GAIN, READ, 1, "stage A: get value which sets AGM sensitivity to peak phase energy slope"),
            cmd_con(AP_STAGE_A_RESID, "GET_ERLE_BAD_GAIN", TYPE_FIXED_7_24, AP_STAGE_A_CMD_GET_ERLE_BAD_GAIN, READ, 1, "stage A: set how steeply AGM drops off when ERLE below threshold"),
            cmd_con(AP_STAGE_A_RESID, "GET_ADEC_FAR_THRESHOLD", TYPE_ENERGY, AP_STAGE_A_CMD_GET_ADEC_FAR_THRESHOLD, READ, 1, "stage A: get far signal energy threshold above which we update AGM"),
            cmd_con(AP_STAGE_A_RESID, "GET_AEC_PEAK_TO_AVERAGE_RATIO", TYPE_ENERGY, AP_STAGE_A_CMD_GET_AEC_PEAK_TO_AVERAGE_RATIO, READ, 1, "stage A: get AEC coefficients peak to average ratio"),
            cmd_con(AP_STAGE_A_RESID, "GET_PHASE_POWERS", TYPE_ENERGY, AP_STAGE_A_CMD_GET_PHASE_POWERS, READ, ADEC_READ_PHASE_POWER_CHUNK_SIZE/sizeof(uint32_t)/2, "stage A: get 5 phase powers (240 samples per phase) used in delay estimation from the index set."),
            cmd_con(AP_STAGE_A_RESID, "GET_PHASE_POWER_INDEX", TYPE_UINT32, AP_STAGE_A_CMD_GET_PHASE_POWER_INDEX, READ, 1, "stage A: get GERLE gain (how strongly it responds either side of mid point)"),
            cmd_con(AP_STAGE_A_RESID, "GET_LOCKER_ENABLED", TYPE_UINT32, AP_STAGE_A_CMD_GET_LOCKER_ENABLED, READ, 1, "stage A: get locker delay detection and control enabled enabled: 0: off, 1: on"),

            cmd_con(AP_STAGE_A_RESID, "GET_LOCKER_STATE", TYPE_UINT32, AP_STAGE_A_CMD_GET_LOCKER_STATE, READ, 1, "stage A: get locker state"),
            cmd_con(AP_STAGE_A_RESID, "GET_MIN_RX_TIME_STAGE_A", TYPE_TICKS, AP_STAGE_A_CMD_GET_MIN_TIME_RX, READ, 1, "stage A min rx time per frame"),
            cmd_con(AP_STAGE_A_RESID, "GET_MAX_RX_TIME_STAGE_A", TYPE_TICKS, AP_STAGE_A_CMD_GET_MAX_TIME_RX, READ, 1, "stage A max rx time per frame"),
            cmd_con(AP_STAGE_A_RESID, "GET_MIN_CONTROL_TIME_STAGE_A", TYPE_TICKS, AP_STAGE_A_CMD_GET_MIN_TIME_CONTROL, READ, 1, "stage A min control time per frame"),
            cmd_con(AP_STAGE_A_RESID, "GET_MAX_CONTROL_TIME_STAGE_A", TYPE_TICKS, AP_STAGE_A_CMD_GET_MAX_TIME_CONTROL, READ, 1, "stage A max control time per frame"),
            cmd_con(AP_STAGE_A_RESID, "GET_MIN_DSP_TIME_STAGE_A", TYPE_TICKS, AP_STAGE_A_CMD_GET_MIN_TIME_DSP, READ, 1, "stage A min dsp time per frame"),
            cmd_con(AP_STAGE_A_RESID, "GET_MAX_DSP_TIME_STAGE_A", TYPE_TICKS, AP_STAGE_A_CMD_GET_MAX_TIME_DSP, READ, 1, "stage A max dsp time per frame"),
            cmd_con(AP_STAGE_A_RESID, "GET_MIN_TX_TIME_STAGE_A", TYPE_TICKS, AP_STAGE_A_CMD_GET_MIN_TIME_TX, READ, 1, "stage A min tx time per frame"),
            cmd_con(AP_STAGE_A_RESID, "GET_MAX_TX_TIME_STAGE_A", TYPE_TICKS, AP_STAGE_A_CMD_GET_MAX_TIME_TX, READ, 1, "stage A max tx time per frame"),
            cmd_con(AP_STAGE_A_RESID, "GET_MIN_IDLE_TIME_STAGE_A", TYPE_TICKS, AP_STAGE_A_CMD_GET_MIN_TIME_IDLE, READ, 1, "stage A min idle time per frame"),
            cmd_con(AP_STAGE_A_RESID, "GET_MAX_IDLE_TIME_STAGE_A", TYPE_TICKS, AP_STAGE_A_CMD_GET_MAX_TIME_IDLE, READ, 1, "stage A max idle time per frame"),

            cmd_con(AP_STAGE_A_RESID, "GET_LOCKER_NUM_BAD_FRAMES_THRESHOLD", TYPE_UINT32, AP_STAGE_A_CMD_GET_LOCKER_NUM_BAD_FRAMES_THRESHOLD, READ, 1, "stage A: get no. of bad peak to avg erle frames that locker sees before it triggers adec. Default: 666"),
            cmd_con(AP_STAGE_A_RESID, "GET_LOCKER_DELAY_SETPOINT_ENABLED", TYPE_UINT32, AP_STAGE_A_CMD_GET_LOCKER_DELAY_SETPOINT_ENABLED, READ, 1, "Get delay setpoint enabled"),
            cmd_con(AP_STAGE_A_RESID, "GET_LOCKER_DELAY_SETPOINT_SAMPLES", TYPE_UINT32, AP_STAGE_A_CMD_GET_LOCKER_DELAY_SETPOINT_SAMPLES, READ, 1, "Get delay setpoint samples"),
            cmd_con(AP_STAGE_A_RESID, "GET_LOCKER_DELAY_SETPOINT_DIRECTION", TYPE_UINT32, AP_STAGE_A_CMD_GET_LOCKER_DELAY_SETPOINT_DIRECTION, READ, 1, "Get delay setpoint direction"),
            cmd_con(AP_STAGE_A_RESID, "GET_ADEC_PEAK_TO_AVERAGE_GOOD_AEC", TYPE_ENERGY, AP_STAGE_A_CMD_GET_ADEC_PEAK_TO_AVERAGE_GOOD_AEC, READ, 1, "stage A: get the peak to average ratio that is considered good when in normal AEC mode"),

            cmd_con(AP_STAGE_A_RESID, "SET_LOCKER_NUM_BAD_FRAMES_THRESHOLD", TYPE_UINT32, AP_STAGE_A_CMD_SET_LOCKER_NUM_BAD_FRAMES_THRESHOLD, WRITE, 1, "stage A: set no. of bad peak to avg erle frames that locker sees before it triggers adec. Default: 666"),
            cmd_con(AP_STAGE_A_RESID, "SET_LOCKER_DELAY_SETPOINT_ENABLED", TYPE_UINT32, AP_STAGE_A_CMD_SET_LOCKER_DELAY_SETPOINT_ENABLED, WRITE, 1, "Set delay setpoint enabled"),
            cmd_con(AP_STAGE_A_RESID, "SET_LOCKER_DELAY_SETPOINT_SAMPLES", TYPE_UINT32, AP_STAGE_A_CMD_SET_LOCKER_DELAY_SETPOINT_SAMPLES, WRITE, 1, "Set delay setpoint samples"),
            cmd_con(AP_STAGE_A_RESID, "SET_LOCKER_DELAY_SETPOINT_DIRECTION", TYPE_UINT32, AP_STAGE_A_CMD_SET_LOCKER_DELAY_SETPOINT_DIRECTION, WRITE, 1, "Set delay setpoint direction"),
            cmd_con(AP_STAGE_A_RESID, "SET_ADEC_PEAK_TO_AVERAGE_GOOD_AEC", TYPE_ENERGY, AP_STAGE_A_CMD_SET_ADEC_PEAK_TO_AVERAGE_GOOD_AEC, WRITE, 1, "stage A: set the peak to average ratio that is considered good when in normal AEC mode"),

            cmd_con(AP_STAGE_B_RESID, "GET_MIN_RX_TIME_STAGE_B", TYPE_TICKS, AP_STAGE_B_CMD_GET_MIN_TIME_RX, READ, 1, "stage B min rx time per frame"),
            cmd_con(AP_STAGE_B_RESID, "GET_MAX_RX_TIME_STAGE_B", TYPE_TICKS, AP_STAGE_B_CMD_GET_MAX_TIME_RX, READ, 1, "stage B max rx time per frame"),
            cmd_con(AP_STAGE_B_RESID, "GET_MIN_CONTROL_TIME_STAGE_B", TYPE_TICKS, AP_STAGE_B_CMD_GET_MIN_TIME_CONTROL, READ, 1, "stage B min control time per frame"),
            cmd_con(AP_STAGE_B_RESID, "GET_MAX_CONTROL_TIME_STAGE_B", TYPE_TICKS, AP_STAGE_B_CMD_GET_MAX_TIME_CONTROL, READ, 1, "stage B max control time per frame"),
            cmd_con(AP_STAGE_B_RESID, "GET_MIN_DSP_TIME_STAGE_B", TYPE_TICKS, AP_STAGE_B_CMD_GET_MIN_TIME_DSP, READ, 1, "stage B min dsp time per frame"),
            cmd_con(AP_STAGE_B_RESID, "GET_MAX_DSP_TIME_STAGE_B", TYPE_TICKS, AP_STAGE_B_CMD_GET_MAX_TIME_DSP, READ, 1, "stage B max dsp time per frame"),
            cmd_con(AP_STAGE_B_RESID, "GET_MIN_TX_TIME_STAGE_B", TYPE_TICKS, AP_STAGE_B_CMD_GET_MIN_TIME_TX, READ, 1, "stage B min tx time per frame"),
            cmd_con(AP_STAGE_B_RESID, "GET_MAX_TX_TIME_STAGE_B", TYPE_TICKS, AP_STAGE_B_CMD_GET_MAX_TIME_TX, READ, 1, "stage B max tx time per frame"),
            cmd_con(AP_STAGE_B_RESID, "GET_MIN_IDLE_TIME_STAGE_B", TYPE_TICKS, AP_STAGE_B_CMD_GET_MIN_TIME_IDLE, READ, 1, "stage B min idle time per frame"),
            cmd_con(AP_STAGE_B_RESID, "GET_MAX_IDLE_TIME_STAGE_B", TYPE_TICKS, AP_STAGE_B_CMD_GET_MAX_TIME_IDLE, READ, 1, "stage B max idle time per frame"),

            cmd_con(AP_STAGE_C_RESID, "GET_MIN_RX_TIME_STAGE_C", TYPE_TICKS, AP_STAGE_C_CMD_GET_MIN_TIME_RX, READ, 1, "stage C min rx time per frame"),
            cmd_con(AP_STAGE_C_RESID, "GET_MAX_RX_TIME_STAGE_C", TYPE_TICKS, AP_STAGE_C_CMD_GET_MAX_TIME_RX, READ, 1, "stage C max rx time per frame"),
            cmd_con(AP_STAGE_C_RESID, "GET_MIN_CONTROL_TIME_STAGE_C", TYPE_TICKS, AP_STAGE_C_CMD_GET_MIN_TIME_CONTROL, READ, 1, "stage C min control time per frame"),
            cmd_con(AP_STAGE_C_RESID, "GET_MAX_CONTROL_TIME_STAGE_C", TYPE_TICKS, AP_STAGE_C_CMD_GET_MAX_TIME_CONTROL, READ, 1, "stage C max control time per frame"),
            cmd_con(AP_STAGE_C_RESID, "GET_MIN_DSP_TIME_STAGE_C", TYPE_TICKS, AP_STAGE_C_CMD_GET_MIN_TIME_DSP, READ, 1, "stage C min dsp time per frame"),
            cmd_con(AP_STAGE_C_RESID, "GET_MAX_DSP_TIME_STAGE_C", TYPE_TICKS, AP_STAGE_C_CMD_GET_MAX_TIME_DSP, READ, 1, "stage C max dsp time per frame"),
            cmd_con(AP_STAGE_C_RESID, "GET_MIN_TX_TIME_STAGE_C", TYPE_TICKS, AP_STAGE_C_CMD_GET_MIN_TIME_TX, READ, 1, "stage C min tx time per frame"),
            cmd_con(AP_STAGE_C_RESID, "GET_MAX_TX_TIME_STAGE_C", TYPE_TICKS, AP_STAGE_C_CMD_GET_MAX_TIME_TX, READ, 1, "stage C max tx time per frame"),
            cmd_con(AP_STAGE_C_RESID, "GET_MIN_IDLE_TIME_STAGE_C", TYPE_TICKS, AP_STAGE_C_CMD_GET_MIN_TIME_IDLE, READ, 1, "stage C min idle time per frame"),
            cmd_con(AP_STAGE_C_RESID, "GET_MAX_IDLE_TIME_STAGE_C", TYPE_TICKS, AP_STAGE_C_CMD_GET_MAX_TIME_IDLE, READ, 1, "stage C max idle time per frame"),
            cmd_con(AGC_RESID, "GET_GAIN_CH0_AGC", TYPE_FIXED_16_16, AGC_CMD_GET_GAIN, READ, 1, "get gain for channel 0"),
            cmd_con(AGC_RESID, "GET_GAIN_CH1_AGC", TYPE_FIXED_16_16, AGC_CMD_GET_GAIN, READ, 1, "get gain for channel 1"),
            cmd_con(AGC_RESID, "GET_MAX_GAIN_CH0_AGC", TYPE_FIXED_16_16, AGC_CMD_GET_MAX_GAIN, READ, 1, "get max gain for channel 0"),
            cmd_con(AGC_RESID, "GET_MAX_GAIN_CH1_AGC", TYPE_FIXED_16_16, AGC_CMD_GET_MAX_GAIN, READ, 1, "get max gain for channel 1"),
            cmd_con(AGC_RESID, "GET_MIN_GAIN_CH0_AGC", TYPE_FIXED_16_16, AGC_CMD_GET_MIN_GAIN, READ, 1, "get min gain for channel 0"),
            cmd_con(AGC_RESID, "GET_MIN_GAIN_CH1_AGC", TYPE_FIXED_16_16, AGC_CMD_GET_MIN_GAIN, READ, 1, "get min gain for channel 1"),
            cmd_con(AGC_RESID, "GET_UPPER_THRESHOLD_CH0_AGC", TYPE_FIXED_1_31, AGC_CMD_GET_DESIRED_UPPER_THRESHOLD, READ, 1, "get upper threshold of desired level for channel 0"),
            cmd_con(AGC_RESID, "GET_UPPER_THRESHOLD_CH1_AGC", TYPE_FIXED_1_31, AGC_CMD_GET_DESIRED_UPPER_THRESHOLD, READ, 1, "get upper threshold of desired level for channel 1"),
            cmd_con(AGC_RESID, "GET_LOWER_THRESHOLD_CH0_AGC", TYPE_FIXED_1_31, AGC_CMD_GET_DESIRED_LOWER_THRESHOLD, READ, 1, "get lower threshold of desired level for channel 0"),
            cmd_con(AGC_RESID, "GET_LOWER_THRESHOLD_CH1_AGC", TYPE_FIXED_1_31, AGC_CMD_GET_DESIRED_LOWER_THRESHOLD, READ, 1, "get lower threshold of desired level for channel 1"),
            cmd_con(AGC_RESID, "GET_ADAPT_CH0_AGC", TYPE_UINT32, AGC_CMD_GET_ADAPT, READ, 1, "get AGC adaptation for channel 0"),
            cmd_con(AGC_RESID, "GET_ADAPT_CH1_AGC", TYPE_UINT32, AGC_CMD_GET_ADAPT, READ, 1, "get AGC adaptation for channel 1"),
            cmd_con(AGC_RESID, "GET_ADAPT_ON_VAD_CH0_AGC", TYPE_UINT32, AGC_CMD_GET_ADAPT_ON_VAD, READ, 1, "get AGC adaptation using VAD data for channel 0"),
            cmd_con(AGC_RESID, "GET_ADAPT_ON_VAD_CH1_AGC", TYPE_UINT32, AGC_CMD_GET_ADAPT_ON_VAD, READ, 1, "get AGC adaptation using VAD data for channel 1"),

            cmd_con(AGC_RESID, "GET_SOFT_CLIPPING_CH0_AGC", TYPE_UINT32, AGC_CMD_GET_SOFT_CLIPPING, READ, 1, "get AGC soft clipping for channel 0"),
            cmd_con(AGC_RESID, "GET_SOFT_CLIPPING_CH1_AGC", TYPE_UINT32, AGC_CMD_GET_SOFT_CLIPPING, READ, 1, "get AGC soft clipping  for channel 1"),
            cmd_con(AGC_RESID, "GET_LC_ENABLED_CH0_AGC", TYPE_UINT32, AGC_CMD_GET_LC_ENABLED, READ, 1, "get loss control enable for channel 0"),
            cmd_con(AGC_RESID, "GET_LC_ENABLED_CH1_AGC", TYPE_UINT32, AGC_CMD_GET_LC_ENABLED, READ, 1, "get loss control enable for channel 1"),

            cmd_con(AGC_RESID, "GET_INCREMENT_GAIN_STEPSIZE_CH0_AGC", TYPE_FIXED_16_16, AGC_CMD_GET_GAIN_INCREMENT_STEPSIZE, READ, 1, "get stepsize with which gain is incremented for AGC ch0"),
            cmd_con(AGC_RESID, "GET_INCREMENT_GAIN_STEPSIZE_CH1_AGC", TYPE_FIXED_16_16, AGC_CMD_GET_GAIN_INCREMENT_STEPSIZE, READ, 1, "get stepsize with which gain is incremented for AGC ch1"),
            cmd_con(AGC_RESID, "GET_DECREMENT_GAIN_STEPSIZE_CH0_AGC", TYPE_FIXED_16_16, AGC_CMD_GET_GAIN_DECREMENT_STEPSIZE, READ, 1, "get stepsize with which gain is decremented for AGC ch0"),
            cmd_con(AGC_RESID, "GET_DECREMENT_GAIN_STEPSIZE_CH1_AGC", TYPE_FIXED_16_16, AGC_CMD_GET_GAIN_DECREMENT_STEPSIZE, READ, 1, "get stepsize with which gain is decremented for AGC ch1"),

            cmd_con(IC_RESID,      "GET_BYPASS_IC", TYPE_INT32, IC_CMD_GET_BYPASS, READ, 1, "IC: get bypass state"),
            cmd_con(IC_RESID,      "GET_X_ENERGY_DELTA_IC", TYPE_ENERGY, IC_CMD_GET_X_ENERGY_DELTA, READ, 1, "IC: get X energy delta"),
            cmd_con(IC_RESID,      "GET_X_ENERGY_GAMMA_LOG2_IC", TYPE_UINT32, IC_CMD_GET_X_ENERGY_GAMMA_LOG2, READ, 1, "IC: get X energy gamma log2"),
            cmd_con(IC_RESID,      "GET_FORCED_MU_VALUE_IC", TYPE_FIXED_1_31, IC_CMD_GET_FORCED_MU_VALUE, READ, 1, "IC: get forced mu value"),
            cmd_con(IC_RESID,      "GET_ADAPTATION_CONFIG_IC", TYPE_INT32, IC_CMD_GET_ADAPTION_CONFIG, READ, 1, "IC: get adaptation config"),
            cmd_con(IC_RESID,      "GET_SIGMA_ALPHA_IC", TYPE_UINT32, IC_CMD_GET_SIGMA_ALPHA, READ, 1, "IC: get adaptation config"),
            cmd_con(IC_RESID,      "GET_PHASES_IC", TYPE_UINT32, IC_CMD_GET_PHASES, READ, 1, "IC: get phases"),
            cmd_con(IC_RESID,      "GET_PROC_FRAME_BINS_IC", TYPE_UINT32, IC_CMD_GET_PROC_FRAME_BINS, READ, 1, "IC: get proc frame bins"),
            cmd_con(IC_RESID,      "GET_FILTER_COEFFICIENTS_IC", TYPE_UINT32, IC_CMD_GET_FILTER_COEFFICIENTS, READ, IC_COEFFICIENT_CHUNK_SIZE/sizeof(uint32_t), "IC: get filter coefficients"),
            cmd_con(IC_RESID,      "GET_COEFFICIENT_INDEX_IC", TYPE_UINT32, IC_CMD_GET_COEFFICIENT_INDEX, READ, 1, "IC: get coefficient index"),

            cmd_con(SUP_RESID,      "GET_BYPASS_SUP", TYPE_INT32, SUP_CMD_GET_BYPASS, READ, 1, "SUP: get bypass"),
            cmd_con(SUP_RESID,      "GET_ENABLED_AES", TYPE_INT32, SUP_CMD_GET_ECHO_SUPPRESSION_ENABLED, READ, 1, "SUP: get echo suppresion enabled"),
            cmd_con(SUP_RESID,      "GET_ENABLED_NS", TYPE_INT32, SUP_CMD_GET_NOISE_SUPPRESSION_ENABLED, READ, 1, "SUP: get noise suppresion enabled"),
            cmd_con(SUP_RESID,      "GET_NOISE_FLOOR_NS", TYPE_FIXED_0_32, SUP_CMD_GET_NOISE_MCRA_NOISE_FLOOR, READ, 1, "SUP: get noise suppression noise floor"),


            cmd_con(AEC_RESID,      "SET_BYPASS_AEC", TYPE_INT32, AEC_CMD_SET_BYPASS, WRITE, 1, "AEC: set bypass"),
            cmd_con(AEC_RESID,      "SET_X_ENERGY_DELTA_AEC", TYPE_ENERGY, AEC_CMD_SET_X_ENERGY_DELTA, WRITE, 1, "AEC: set X energy delta"),
            cmd_con(AEC_RESID,      "SET_X_ENERGY_GAMMA_LOG2_AEC", TYPE_INT32, AEC_CMD_SET_X_ENERGY_GAMMA_LOG2, WRITE, 1, "AEC: set X energy gamma log2"),
            cmd_con(AEC_RESID,      "SET_FORCED_MU_VALUE_AEC", TYPE_FIXED_1_31, AEC_CMD_SET_FORCED_MU_VALUE, WRITE, 1, "AEC: set forced mu value"),
            cmd_con(AEC_RESID,      "SET_ADAPTATION_CONFIG_AEC", TYPE_INT32, AEC_CMD_SET_ADAPTION_CONFIG, WRITE, 1, "AEC: set adaptation config"),
            cmd_con(AEC_RESID,      "SET_MU_SCALAR_AEC", TYPE_FIXED_8_24, AEC_CMD_SET_MU_SCALAR, WRITE, 1, "AEC: set mu_scalar"),
            cmd_con(AEC_RESID,      "SET_MU_LIMITS_AEC", TYPE_FIXED_1_31, AEC_CMD_SET_MU_LIMITS, WRITE, 2, "AEC: set mu_high and mu_low"),
            cmd_con(AEC_RESID,      "SET_SIGMA_ALPHAS_AEC", TYPE_UINT32, AEC_CMD_SET_SIGMA_ALPHAS, WRITE, 3, "AEC: set sigma alphas"),
            cmd_con(AEC_RESID,      "RESET_FILTER_AEC", TYPE_UINT8, AEC_CMD_RESET_FILTER, WRITE, 1, "AEC: reset filter"),

            cmd_con(AEC_RESID,      "SET_COEFF_INDEX_AEC", TYPE_UINT32, AEC_CMD_SET_COEFFICIENT_INDEX, WRITE, 1, "AEC: set coefficient index"),

            cmd_con(AP_STAGE_A_RESID, "SET_DELAY_DIRECTION", TYPE_UINT32, AP_STAGE_A_CMD_SET_DELAY_DIRECTION, WRITE, 1, "set configurable delay direction: 0: delay references, 1: delay mics "),
            cmd_con(AP_CONTROL_RESID, "SET_DELAY_SAMPLES", TYPE_UINT32, AP_CONTROL_CMD_SET_DELAY_SAMPLES, WRITE, 1, "CONTROL: set configurable delay in samples"),
            cmd_con(AP_STAGE_A_RESID, "SET_DELAY_ESTIMATOR_ENABLED", TYPE_UINT32, AP_STAGE_A_CMD_SET_DELAY_ESTIMATOR_ENABLED, WRITE, 1, "set delay estimator enabled"),
            cmd_con(AP_STAGE_A_RESID, "SET_MIC_SHIFT_SATURATE", TYPE_UINT32, AP_STAGE_A_CMD_SET_MIC_SHIFT_SATURATE, WRITE, 2, "CONTROL: set the shift value and saturation (1=enable) to be applied to the input mic samples"),
            cmd_con(AP_STAGE_A_RESID, "SET_ALT_ARCH_ENABLED", TYPE_UINT32, AP_STAGE_A_CMD_SET_ALT_ARCH_ENABLED, WRITE, 1, "stage A: Set state of xvf3510 alternate architecture setting: 0: normal, 1: alt-arch"),


            cmd_con(AP_STAGE_A_RESID, "SET_ADEC_ENABLED", TYPE_UINT32, AP_STAGE_A_CMD_SET_ADEC_ENABLED, WRITE, 1, "set delay estimator controller enabled: 0: off, 1: on"),
            cmd_con(AP_STAGE_A_RESID, "SET_MANUAL_ADEC_CYCLE_TRIGGER", TYPE_UINT32, AP_STAGE_A_CMD_SET_MANUAL_ADEC_CYCLE_TRIGGER, WRITE, 1, "trigger a delay estimate + delay configure cycle when 1 is written"),
            cmd_con(AP_STAGE_A_RESID, "SET_ERLE_BAD_BITS", TYPE_FIXED_7_24, AP_STAGE_A_CMD_SET_ERLE_BAD_BITS, WRITE, 1, "set ERLE threshold at which AGM is decreased"),
            cmd_con(AP_STAGE_A_RESID, "SET_ERLE_GOOD_BITS", TYPE_FIXED_7_24, AP_STAGE_A_CMD_SET_ERLE_GOOD_BITS, WRITE, 1, "set ERLE threshold at which AGM is increased"),
            cmd_con(AP_STAGE_A_RESID, "SET_ERLE_BAD_GAIN", TYPE_FIXED_7_24, AP_STAGE_A_CMD_SET_ERLE_BAD_GAIN, WRITE, 1, "set how strongly AGM is updated when ERLE below threshold"),
            cmd_con(AP_STAGE_A_RESID, "SET_PEAK_PHASE_ENERGY_TREND_GAIN", TYPE_FIXED_7_24, AP_STAGE_A_CMD_SET_PEAK_PHASE_ENERGY_TREND_GAIN, WRITE, 1, "set how strongly AGM is updated by the peak phase slope"),
            cmd_con(AP_STAGE_A_RESID, "SET_ADEC_FAR_THRESHOLD", TYPE_ENERGY, AP_STAGE_A_CMD_SET_ADEC_FAR_THRESHOLD, WRITE, 1, "set energy threshold of far signal above which AGM is updated"),
            cmd_con(AP_STAGE_A_RESID, "SET_PHASE_POWER_INDEX", TYPE_UINT32, AP_STAGE_A_CMD_SET_PHASE_POWER_INDEX, WRITE, 1, "set index for reading phase powers"),
            cmd_con(AP_STAGE_A_RESID, "SET_LOCKER_ENABLED", TYPE_UINT32, AP_STAGE_A_CMD_SET_LOCKER_ENABLED, WRITE, 1, "stage A: set locker delay detection and control enabled enabled: 0: off, 1: on"),

            cmd_con(AP_STAGE_A_RESID, "RESET_TIME_STAGE_A", TYPE_UINT8, AP_STAGE_A_CMD_RESET_TIME, WRITE, 1, "reset stage A frame time"),
            cmd_con(AP_STAGE_B_RESID, "RESET_TIME_STAGE_B", TYPE_UINT8, AP_STAGE_B_CMD_RESET_TIME, WRITE, 1, "reset stage B frame time"),
            cmd_con(AP_STAGE_C_RESID, "RESET_TIME_STAGE_C", TYPE_UINT8, AP_STAGE_C_CMD_RESET_TIME, WRITE, 1, "reset stage C frame time"),

            cmd_con(AGC_RESID, "SET_GAIN_CH0_AGC", TYPE_FIXED_16_16, AGC_CMD_SET_GAIN, WRITE, 1, "set gain for channel 0"),
            cmd_con(AGC_RESID, "SET_GAIN_CH1_AGC", TYPE_FIXED_16_16, AGC_CMD_SET_GAIN, WRITE, 1, "set gain for channel 1"),
            cmd_con(AGC_RESID, "SET_MAX_GAIN_CH0_AGC", TYPE_FIXED_16_16, AGC_CMD_SET_MAX_GAIN, WRITE, 1, "set max gain for channel 0"),
            cmd_con(AGC_RESID, "SET_MAX_GAIN_CH1_AGC", TYPE_FIXED_16_16, AGC_CMD_SET_MAX_GAIN, WRITE, 1, "set gain for channel 1"),
            cmd_con(AGC_RESID, "SET_MAX_GAIN_CH1_AGC", TYPE_FIXED_16_16, AGC_CMD_SET_MAX_GAIN, WRITE, 1, "set max gain for channel 1"),
            cmd_con(AGC_RESID, "SET_MIN_GAIN_CH0_AGC", TYPE_FIXED_16_16, AGC_CMD_SET_MIN_GAIN, WRITE, 1, "set min gain for channel 0"),
            cmd_con(AGC_RESID, "SET_MIN_GAIN_CH1_AGC", TYPE_FIXED_16_16, AGC_CMD_SET_MIN_GAIN, WRITE, 1, "set min gain for channel 1"),
            cmd_con(AGC_RESID, "SET_UPPER_THRESHOLD_CH0_AGC", TYPE_FIXED_1_31, AGC_CMD_SET_DESIRED_UPPER_THRESHOLD, WRITE, 1, "set upper threshold of desired level for channel 0"),
            cmd_con(AGC_RESID, "SET_UPPER_THRESHOLD_CH1_AGC", TYPE_FIXED_1_31, AGC_CMD_SET_DESIRED_UPPER_THRESHOLD, WRITE, 1, "set upper threshold of desired level for channel 1"),
            cmd_con(AGC_RESID, "SET_LOWER_THRESHOLD_CH0_AGC", TYPE_FIXED_1_31, AGC_CMD_SET_DESIRED_LOWER_THRESHOLD, WRITE, 1, "set lower threshold of desired level for channel 0"),
            cmd_con(AGC_RESID, "SET_LOWER_THRESHOLD_CH1_AGC", TYPE_FIXED_1_31, AGC_CMD_SET_DESIRED_LOWER_THRESHOLD, WRITE, 1, "set lower threshold of desired level for channel 1"),
            cmd_con(AGC_RESID, "SET_ADAPT_CH0_AGC", TYPE_UINT32, AGC_CMD_SET_ADAPT, WRITE, 1, "set AGC adaptation for channel 0"),
            cmd_con(AGC_RESID, "SET_ADAPT_CH1_AGC", TYPE_UINT32, AGC_CMD_SET_ADAPT, WRITE, 1, "set AGC adaptation for channel 1"),
            cmd_con(AGC_RESID, "SET_ADAPT_ON_VAD_CH0_AGC", TYPE_UINT32, AGC_CMD_SET_ADAPT_ON_VAD, WRITE, 1, "set AGC adaptation using VAD data for channel 0"),
            cmd_con(AGC_RESID, "SET_ADAPT_ON_VAD_CH1_AGC", TYPE_UINT32, AGC_CMD_SET_ADAPT_ON_VAD, WRITE, 1, "set AGC adaptation using VAD data for channel 1"),

            cmd_con(AGC_RESID, "SET_SOFT_CLIPPING_CH0_AGC", TYPE_UINT32, AGC_CMD_SET_SOFT_CLIPPING, WRITE, 1, "set AGC soft clipping for channel 0"),
            cmd_con(AGC_RESID, "SET_SOFT_CLIPPING_CH1_AGC", TYPE_UINT32, AGC_CMD_SET_SOFT_CLIPPING, WRITE, 1, "set AGC soft clipping for channel 1"),
            cmd_con(AGC_RESID, "SET_LC_ENABLED_CH0_AGC", TYPE_UINT32, AGC_CMD_SET_LC_ENABLED, WRITE, 1, "enable loss control for channel 0"),
            cmd_con(AGC_RESID, "SET_LC_ENABLED_CH1_AGC", TYPE_UINT32, AGC_CMD_SET_LC_ENABLED, WRITE, 1, "enable loss control for channel 1"),
            cmd_con(AGC_RESID, "SET_INCREMENT_GAIN_STEPSIZE_CH0_AGC", TYPE_FIXED_16_16, AGC_CMD_SET_GAIN_INCREMENT_STEPSIZE, WRITE, 1, "set stepsize with which gain is incremented for AGC ch0"),
            cmd_con(AGC_RESID, "SET_INCREMENT_GAIN_STEPSIZE_CH1_AGC", TYPE_FIXED_16_16, AGC_CMD_SET_GAIN_INCREMENT_STEPSIZE, WRITE, 1, "set stepsize with which gain is incremented for AGC ch1"),
            cmd_con(AGC_RESID, "SET_DECREMENT_GAIN_STEPSIZE_CH0_AGC", TYPE_FIXED_16_16, AGC_CMD_SET_GAIN_DECREMENT_STEPSIZE, WRITE, 1, "set stepsize with which gain is decremented for AGC ch0"),
            cmd_con(AGC_RESID, "SET_DECREMENT_GAIN_STEPSIZE_CH1_AGC", TYPE_FIXED_16_16, AGC_CMD_SET_GAIN_DECREMENT_STEPSIZE, WRITE, 1, "set stepsize with which gain is decremented for AGC ch1"),

            cmd_con(IC_RESID,      "SET_BYPASS_IC", TYPE_INT32, IC_CMD_SET_BYPASS, WRITE, 1, "IC: set bypass"),
            cmd_con(IC_RESID,      "SET_X_ENERGY_DELTA_IC", TYPE_ENERGY, IC_CMD_SET_X_ENERGY_DELTA, WRITE, 1, "IC: set X energy delta"),
            cmd_con(IC_RESID,      "SET_X_ENERGY_GAMMA_LOG2_IC", TYPE_UINT32, IC_CMD_SET_X_ENERGY_GAMMA_LOG2, WRITE, 1, "IC: set X energy gamma log2"),
            cmd_con(IC_RESID,      "SET_FORCED_MU_VALUE_IC", TYPE_FIXED_1_31, IC_CMD_SET_FORCED_MU_VALUE, WRITE, 1, "IC: set forced mu value"),
            cmd_con(IC_RESID,      "SET_ADAPTATION_CONFIG_IC", TYPE_INT32, IC_CMD_SET_ADAPTION_CONFIG, WRITE, 1, "IC: set adaptation config"),
            cmd_con(IC_RESID,      "SET_SIGMA_ALPHA_IC", TYPE_UINT32, IC_CMD_SET_SIGMA_ALPHA, WRITE, 1, "IC: set adaptation config"),
            cmd_con(IC_RESID,      "SET_COEFFICIENT_INDEX_IC", TYPE_UINT32, IC_CMD_SET_COEFFICIENT_INDEX, WRITE, 1, "IC: set coefficient index"),
            cmd_con(IC_RESID,      "RESET_FILTER_IC", TYPE_UINT8, IC_CMD_RESET_FILTER, WRITE, 1, "IC: reset filter"),
            cmd_con(IC_RESID,      "SET_CH1_BEAMFORM_ENABLE", TYPE_UINT8, IC_CMD_SET_CH1_BEAMFORM_ENABLE, WRITE, 1, "set if beamforming is enabled on channel1"),

            cmd_con(SUP_RESID,      "SET_BYPASS_SUP", TYPE_INT32, SUP_CMD_SET_BYPASS, WRITE, 1, "SUP: set bypass"),
            cmd_con(SUP_RESID,      "SET_ENABLED_AES", TYPE_INT32, SUP_CMD_SET_ECHO_SUPPRESSION_ENABLED, WRITE, 1, "SUP: set echo suppresion enabled"),
            cmd_con(SUP_RESID,      "SET_ENABLED_NS", TYPE_INT32, SUP_CMD_SET_NOISE_SUPPRESSION_ENABLED, WRITE, 1, "SUP: set noise suppresion enabled"),
            cmd_con(SUP_RESID,      "SET_NOISE_FLOOR_NS", TYPE_FIXED_0_32, SUP_CMD_SET_NOISE_MCRA_NOISE_FLOOR, WRITE, 1, "SUP: set noise suppression noise floor"),
        };
        total_num_commands = sizeof(cmdspec_ap_local)/sizeof(cmdspec_t);
        cmdspec_ap = (cmdspec_t*)calloc(total_num_commands, sizeof(cmdspec_t));
        memcpy(cmdspec_ap, cmdspec_ap_local, total_num_commands * sizeof(cmdspec_t));
    }
}

void open_device() {
#if USE_I2C
    setup_err = i2c_setup();
#elif USE_USB
    #ifdef __ANDROID__
    setup_err = 0;
    #else
    setup_err = usb_setup();
    #endif // __ANDROID__
#elif JSON_ONLY
    // do nothing
#endif
}

void vfctrl_set_vendor_id(int vendor_id) {
    g_vendor_id = vendor_id;
}

void vfctrl_set_product_id(int product_id) {
    g_product_id = product_id;
}

void format_version(void* data_out_ptr, char *version_string)
{
    int32_t version = *(int32_t*)data_out_ptr;
    int major = (version & 0xff000000) >> 24;
    int minor = (version & 0x00f00000) >> 20;
    int patch = (version & 0x000f0000) >> 16;
    sprintf(version_string, "v%d.%d.%d", major, minor, patch);
}

int vfctrl_check_version(unsigned print_version_only) {
#if !JSON_ONLY
    LOCK_MUTEX
    char good_version[50];
    char firmware_version[50];
    char xgit_hash[50];
    sprintf(good_version, "v%d.%d.%d", DEVICE_VERSION_MAJOR, DEVICE_VERSION_MINOR, DEVICE_VERSION_PATCH);

    int_float cmd_output[CMD_MAX_BYTES];

    open_device();

    // GET_VERSION
    cmdspec_t cmd;;
    int ret = vfctrl_get_cmdspec(1, "GET_VERSION", &cmd, 0);
    if(ret != 0) {
        exit(ret);
    }
    get_struct_val_from_device(cmd, cmd_output);
    format_version(cmd_output, firmware_version);

    // if the version must be printed only, retrieve the XGIT hash as well
    if (print_version_only) {
        // GET_BLD_XGIT_HASH
        ret = vfctrl_get_cmdspec(1, "GET_BLD_XGIT_HASH", &cmd, 0);
        if(ret != 0) {
            exit(ret);
        }
        get_struct_val_from_device(cmd, cmd_output);
        update_read_results(cmd, cmd_output, xgit_hash);

        printf("Firmware version: %s - build identifier %s\n", firmware_version, (char*) xgit_hash);
    } else {
        if (strcmp(firmware_version, good_version) != 0) {
            printf("Error: Your firmware version (%s) is incompatible with this vfctrl version (%s).\n",
                firmware_version, good_version);
            printf("To disable this check, use --no-check-version or -n\n");
            host_shutdown(1);
        }
    }
    UNLOCK_MUTEX
#endif // !JSON_ONLY
    return 0;
}

int vfctrl_check_run_status(cmdspec_t cmd) {
    LOCK_MUTEX
    populate_cmd_table();
    open_device();

    if(setup_err != 0)
    {
        printf("control initialization returned error");
        host_shutdown(-1);
    }
    void *data_out_ptr = calloc(cmd.app_read_result_size, 1);

    int ret = get_struct_val_from_device(cmd, data_out_ptr);

    if(ret != 0)
    {
        host_shutdown(ret);
    }

    uint8_t run_status_val = ((char*)data_out_ptr)[0];

    if (run_status_val != RUN_FACTORY_DATA_SUCCESS &&\
        run_status_val != RUN_UPGRADE_DATA_SUCCESS)
    {
        char output_str[50];
        memset(output_str,'\0', 50);
        format_run_status(&run_status_val, output_str);
        printf("Warning: RUN STATUS is%s, the data partition has not been read\n", output_str);
    }
    free(data_out_ptr);
    UNLOCK_MUTEX
    return 0;
}

char* vfctrl_print_help(unsigned full)
{
    LOCK_MUTEX
    populate_cmd_table();
#if USE_USB
    char * app_name = "vfctrl_usb";
#elif USE_I2C
    char * app_name = "vfctrl_i2c";
#elif JSON_ONLY
    char * app_name = "vfctrl_json";
#endif
    char * ret_string = print_help(app_name, cmdspec_ap, total_num_commands, full);
    printf("\n--------------------------------------\n\n");
    printf("Version information:\n");
    printf("Host app version: v%d.%d.%d\n", DEVICE_VERSION_MAJOR, DEVICE_VERSION_MINOR, DEVICE_VERSION_PATCH);
    UNLOCK_MUTEX
    return ret_string;
}

void vfctrl_dump_params(void)
{
    LOCK_MUTEX
    populate_cmd_table();
    open_device();
    for (int i=0; i<total_num_commands; i++) {
        cmdspec_t cmd = cmdspec_ap[i];
        // GET_FILTER_COEFF must be handled separately
        if (cmd.offset == GPIO_CMD_GET_FILTER_COEFF && cmd.resid == GPIO_RESID && strstr(cmd.par_name, "_RAW")==0) {
            UNLOCK_MUTEX // The mutex is locked inside vfctrl_get_filter_coefficients_human_readable()
            vfctrl_get_filter_coefficients_human_readable(&cmd);
            LOCK_MUTEX
        // check if we have command with GET_ at the start of the parameter name and exclude AEC_CMD_GET_FILTER_COEFFICIENTS and IC_CMD_GET_FILTER_COEFFICIENTS
        } else if (strstr(cmd.par_name, "GET_") == cmd.par_name &&\
            (cmd.offset != AEC_CMD_GET_FILTER_COEFFICIENTS || cmd.resid != AEC_RESID) &&\
            (cmd.offset != IC_CMD_GET_FILTER_COEFFICIENTS || cmd.resid != IC_RESID)) {
            const char* a[1] = {cmd.par_name};
            void *outptr = calloc(cmd.app_read_result_size, 1);
            int ret = do_command(cmd, a, outptr, 0);
            if (!ret) {
                char temp_string[TEMP_STR_MAX_CHARS];
                vfctrl_format_read_result(&cmd, outptr, temp_string);
                printf("%s\n", temp_string);
            } else {
                printf("%s: ERROR (%d)\n", cmd.par_name, ret);
            }
            free(outptr);
        }
    }
    UNLOCK_MUTEX
}

int vfctrl_get_cmdspec(int num_args, const char *command, cmdspec_t *cmd_spec, uint8_t log_for_data_partition)
{
    populate_cmd_table();
    int cmd_num = check_command(num_args, command, cmdspec_ap, total_num_commands);
    if(cmd_num < 0)
    {
        return cmd_num;
    }

    //For get aec coefficients and get ic coefficients, get the size of buffer the user needs to allocate
    memcpy(cmd_spec, &cmdspec_ap[cmd_num], sizeof(cmdspec_t));
    return 0;
}

int vfctrl_do_command(cmdspec_t *cmd_spec, const char **command_plus_values, void *data_out_ptr, uint8_t log_for_data_partition)
{
    LOCK_MUTEX
    populate_cmd_table();
    if (!log_for_data_partition) {
        open_device();
    }
    int ret = do_command(*cmd_spec, &command_plus_values[1], data_out_ptr, log_for_data_partition);
    UNLOCK_MUTEX
    return ret;
}
/*
int vfctrl_do_command_write_bytes(cmdspec_t *cmd_spec, uint8_t *write_bytes)
{
    int_float vals[CMD_MAX_BYTES];
    for(uint32_t i=0; i<cmd_spec->num_values; i++)
    {
        vals[i].ui8 = write_bytes[i];
    }
    int ret = set_struct_val_on_device(*cmd_spec, vals); //write to device
    return ret;
}*/

int vfctrl_get_aec_coefficients_to_file(const char* aec_coeffs_file)
{
    LOCK_MUTEX
    populate_cmd_table();
    open_device();
    int ret = get_aec_coefficients(cmdspec_ap, total_num_commands, aec_coeffs_file);
    UNLOCK_MUTEX
    return ret;
}

int vfctrl_get_ic_coefficients_to_file(const char* ic_coeffs_file)
{
    LOCK_MUTEX
    populate_cmd_table();
    open_device();
    int ret = get_ic_coefficients(cmdspec_ap, total_num_commands, ic_coeffs_file);
    UNLOCK_MUTEX
    return ret;
}

int vfctrl_format_read_result(cmdspec_t *cmd_spec_ptr, void* data_out_ptr, char* output_string) {
    cmdspec_t cmd_spec = *cmd_spec_ptr;
    printf("%s:", cmd_spec.par_name);
    output_string[0] = '\0';
    // Special cases
    if (strcmp("GET_VERSION", cmd_spec.par_name) == 0) {
        format_version(data_out_ptr, output_string);
        return 0;
    }
    //get_erle is a special command since the cmd spec says 2 values (mic and ref enery)need to be read to the device but the output is just 1 value (erle in dB).
    if((strcmp("GET_ERLE_CH0_AEC", cmd_spec.par_name) == 0) || (strcmp("GET_ERLE_CH1_AEC", cmd_spec.par_name) == 0))
    {
        sprintf(output_string, "%f dB", *((float*)data_out_ptr));
        return 0;
    }
    if(strcmp("GET_IO_MAP_AND_SHIFT", cmd_spec.par_name) == 0)
    {
        format_io_map((uint8_t*)data_out_ptr, output_string);
        return 0;
    }
    if(strcmp("GET_RUN_STATUS", cmd_spec.par_name) == 0)
    {
        format_run_status((uint8_t*)data_out_ptr, output_string);
        return 0;
    }

    if(strcmp("GET_LOCKER_STATE", cmd_spec.par_name) == 0)
    {
        format_locker_state((uint32_t*)data_out_ptr, output_string);
        return 0;
    }

   if (strcmp("GET_BLD_MSG", cmd_spec.par_name) == 0 ||
        strcmp("GET_BLD_HOST", cmd_spec.par_name) == 0 ||
        strcmp("GET_BLD_REPO_HASH", cmd_spec.par_name) == 0 ||
        strcmp("GET_BLD_XGIT_VIEW", cmd_spec.par_name) == 0 ||
        strcmp("GET_BLD_XGIT_HASH", cmd_spec.par_name) == 0 ||
        strcmp("GET_BLD_MODIFIED", cmd_spec.par_name) == 0 ||
        strcmp("GET_USB_PRODUCT_STRING", cmd_spec.par_name) == 0 ||
        strcmp("GET_USB_VENDOR_STRING", cmd_spec.par_name) == 0 ||
        strcmp("GET_SERIAL_NUMBER", cmd_spec.par_name) == 0) {
        ((char*)data_out_ptr)[cmd_spec.num_values] = '\0'; // explicit termination ('semi terminated' format of payload where maximum length string comes without terminator)
        uint32_t string_len = MIN(strlen(data_out_ptr), cmd_spec.num_values-1);
        // add a blank space before the string
        memcpy(output_string, " ", 1);
        memcpy(output_string+1, data_out_ptr, string_len);
        output_string[string_len+1] = '\0';
        return 0;
    }

    for(uint32_t i=0; i<cmd_spec.num_values; i++){
        char temp_string[TEMP_STR_MAX_CHARS];
        switch (cmd_spec.type) {
            case TYPE_INT8:
                sprintf(temp_string, " %d ", ((int8_t*)data_out_ptr)[i]);
                break;
            case TYPE_UINT8:
                sprintf(temp_string, " %d ", ((uint8_t*)data_out_ptr)[i]);
                break;
            case TYPE_INT32:
                sprintf(temp_string, " %d ", ((int32_t*)data_out_ptr)[i]);
                break;
            case TYPE_UINT32:
                sprintf(temp_string, " %d ", ((uint32_t*)data_out_ptr)[i]);
                break;
            case TYPE_ENERGY:
                sprintf(temp_string, " %f dB", ((float*)data_out_ptr)[i]);
                break;
            case TYPE_FIXED_0_32:
            case TYPE_FIXED_1_31:
            case TYPE_FIXED_7_24:
            case TYPE_FIXED_8_24:
            case TYPE_FIXED_16_16:
                sprintf(temp_string, " %.4f", ((float*)data_out_ptr)[i]);
                break;
            case TYPE_TICKS:
                sprintf(temp_string, " %i (%.2f ms)", ((uint32_t*)data_out_ptr)[i], (float) (((uint32_t*)data_out_ptr)[i]) / 100000.0 );
                break;
            default:
                sprintf(temp_string, "Error: invalid type %d\n", cmd_spec.type);
                host_shutdown(1);
        }
        strcat(output_string, temp_string);
    }
    return 0;
}

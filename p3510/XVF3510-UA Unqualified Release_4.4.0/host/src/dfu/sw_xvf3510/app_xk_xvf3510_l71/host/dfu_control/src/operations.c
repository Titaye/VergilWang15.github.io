// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#ifdef __xcore__
#include <string.h>
#else
#include <memory.h>
#include <time.h>
#endif

// byte order portability
#ifdef _WIN32
#define htole16(x) (x) // Windows little endian only
#define le32toh(x) (x)
#elif __APPLE__
#include <libkern/OSByteOrder.h>
#define htole16(x) OSSwapHostToLittleInt16(x)
#define le32toh(x) OSSwapLittleToHostInt32(x)
#elif __xcore__
#define htole16(x) (x) // XCore is little endian
#define le32toh(x) (x)
#else
#include <endian.h>
#endif

#include "dfu_commands.h"
#include "labels.h"
#include "sleep.h"
#include "hal.h"
#include "input_reader.h"
#include "operations.h"

extern bool quiet;

static int check_state(enum dfu_state expected)
{
  enum dfu_state state;

  if (hal_read_command(DFU_CMD_GETSTATE, (unsigned char*)&state,
                       sizeof(enum dfu_state)) != 0) {
    return 1;
  }

  // convert from hard little endian order after deserialisation
  state = (enum dfu_state)le32toh(state);

  if (state == DFU_ERROR) {
    fprintf(stderr, "device in dfuERROR state\n");

    struct dfu_getstatus getstatus;
    if (hal_read_command(DFU_CMD_GETSTATUS, (unsigned char*)&getstatus,
                         sizeof(struct dfu_getstatus)) == 0) {
      getstatus.status = (enum dfu_status)le32toh(getstatus.status);
      fprintf(stderr, "status %s\n", status_str(getstatus.status));
    }

    int error_info = 0;
    if (hal_read_command(DFU_CMD_GET_ERROR_INFO, (unsigned char*)&error_info,
                         sizeof(int)) == 0) {
      error_info = le32toh(error_info);
      fprintf(stderr, "info code %u\n", error_info);
    }

    fprintf(stderr, "send CLRSTATUS to attempt recovery\n");
    hal_write_command(DFU_CMD_CLRSTATUS, NULL, 0);
    return 2;
  }

  if (state != expected) {
    fprintf(stderr, "device state is %s (%d), expected %s (%d)\n",
            state_str(state), state, state_str(expected), expected);
    return 3;
  }

  return 0;
}

static int check_status(struct dfu_getstatus *getstatus)
{
  if (hal_read_command(DFU_CMD_GETSTATUS, (unsigned char*)getstatus,
                       sizeof(struct dfu_getstatus)) != 0) {
    return 1;
  }

  // convert from hard little endian order after deserialisation
  getstatus->state = le32toh(getstatus->state);
  getstatus->status = le32toh(getstatus->status);
  getstatus->poll_timeout_msec = le32toh(getstatus->poll_timeout_msec);

  if (getstatus->status != DFU_OK) {
    fprintf(stderr, "error: status was %s when %s expected\n",
            status_str(getstatus->status), status_str(DFU_OK));

    int error_info = 0;
    if (hal_read_command(DFU_CMD_GET_ERROR_INFO, (unsigned char*)&error_info,
                         sizeof(int)) == 0) {
      fprintf(stderr, "info code %d\n", error_info);
    }

    fprintf(stderr, "state %s (%d)\n",
                    state_str(getstatus->state), getstatus->state);

    fprintf(stderr, "send CLRSTATUS to attempt recovery\n");
    hal_write_command(DFU_CMD_CLRSTATUS, NULL, 0);
    return 2;
  }

  if (!quiet)
    printf("poll timeout %u msec\n", getstatus->poll_timeout_msec);

  return 0;
}

int detach_and_bus_reset(void)
{
  if (!quiet)
    printf("detach and bus reset\n");

  if (check_state(APP_IDLE) != 0)
    return 1;

  if (hal_write_command(DFU_CMD_DETACH, NULL, 0) != 0)
    return 2;

  if (check_state(APP_DETACH) != 0)
    return 3;

  if (hal_write_command(DFU_CMD_BUS_RESET, NULL, 0) != 0)
    return 4;

  if (check_state(DFU_IDLE) != 0)
    return 5;

  if (!quiet)
    printf("detach and bus reset successful\n");

  return 0;
}

static int dnload_block(const unsigned char *block, int num_block_bytes,
                        unsigned block_count, unsigned short marker)
{
  struct {
    struct dfu_dnload_header header;
    unsigned char block[DFU_BLOCK_SIZE_MAX_BYTES];
  } payload;

  // note hard little endian order of block number for serialisation
  size_t payload_bytes = num_block_bytes + sizeof(struct dfu_dnload_header);
  payload.header.block_num = htole16(marker | block_count);
  payload.header.pad = 0;
  memcpy(payload.block, block, num_block_bytes);

  if (hal_write_command(DFU_CMD_DNLOAD, (unsigned char*)&payload,
                        payload_bytes) != 0) {
    return 1;
  }

  return 0;
}

static int download_file(const unsigned char *bytes, size_t length,
                         unsigned block_size, unsigned short marker)
{
  size_t byte_count = 0;
  unsigned block_count = 0;
  struct dfu_getstatus getstatus;

  if (!quiet) {
    printf("start download of %d bytes, block size %d, marker 0x%X\n",
           (int)length, block_size, marker); // size_t different in xCORE unit test
  }

  while (byte_count < length) {
    size_t block_bytes = block_size;
    if (length - byte_count < block_size)
      block_bytes = length - byte_count;

    if (!quiet) {
      printf("download block %u, %d bytes\n",
             block_count, (int)block_bytes); // size_t different in xCORE unit test
    }

    if (dnload_block(bytes + byte_count, block_bytes, block_count, marker) != 0)
      return 1;

    do {
      if (check_status(&getstatus) != 0)
        return 2;

      sleep_milliseconds(getstatus.poll_timeout_msec);
    } while (getstatus.state == DFU_DNBUSY);

    if (check_state(DFU_DNLOAD_IDLE) != 0)
      return 3;

    block_count++;
    byte_count += block_bytes;
  }

  if (dnload_block(NULL, 0, 0, marker) != 0)
    return 4;

  do {
    if (check_status(&getstatus) != 0)
      return 5;

    sleep_milliseconds(getstatus.poll_timeout_msec);
  } while (getstatus.state == DFU_MANIFEST);

  if (check_state(DFU_IDLE) != 0)
    return 6;

  return 0;
}

int write_upgrade(struct inputs inputs, unsigned block_size,
                  bool skip_boot_image, bool skip_data_image)
{
  if (!quiet) {
    printf("write upgrade %d boot bytes and %d data bytes\n",
           (int)inputs.boot.length, (int)inputs.data.length);
  }

  if (detach_and_bus_reset() != 0)
    return 1;

  // data first so a failed cycle doesn't leave a good boot
  // that way device only needs to fall back on bad data but not on bad boot
  if (!skip_data_image) {
    if (download_file(inputs.data.bytes, inputs.data.length,
                      block_size, DFU_BLOCK_NUM_DATA_IMAGE_MARKER) != 0)
      return 2;
  }

  if (!skip_boot_image) {
    if (download_file(inputs.boot.bytes, inputs.boot.length,
                      block_size, 0) != 0)
      return 3;
  }

  if (!quiet)
    printf("write upgrade successful\n");

  return 0;
}

int override_spispec(struct inputs inputs)
{
  if (!quiet)
    printf("override spispec (%d bytes)\n", (int)inputs.spispec.length);

  if (hal_write_command(DFU_CMD_OVERRIDE_SPISPEC,
                        inputs.spispec.bytes, inputs.spispec.length) != 0) {
    return 1;
  }

  if (!quiet)
    printf("override spispec successful\n");

  return 0;
}

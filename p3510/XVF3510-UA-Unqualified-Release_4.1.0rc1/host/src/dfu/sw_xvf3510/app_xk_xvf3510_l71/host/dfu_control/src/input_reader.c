// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <memory.h>
#include <errno.h>
#include "suffix_verifier.h"
#include "device_id.h"
#include "input_reader.h"

extern bool quiet;

static size_t file_size(FILE *handle, const char *name)
{
  if (handle == NULL)
    return 0;

  if (fseek(handle, 0, SEEK_END) != 0) {
    fprintf(stderr, "fseek failed on %s (errno %d)\n", name, errno);
    exit(1);
  }

  long length = ftell(handle);

  if (fseek(handle, 0, SEEK_SET) != 0) {
    fprintf(stderr, "fseek failed on %s (errno %d)\n", name, errno);
    exit(1);
  }

  return (size_t)length;
}

static size_t verify_suffix(const unsigned char *bytes, size_t num_bytes,
                            struct device_id device_id)
{
  size_t suffix_length = 0;
  char msg[256];
  int ret;

  ret = verify_dfu_suffix(bytes, num_bytes, device_id.vendor, device_id.product,
                          device_id.bcddevice, &suffix_length, msg);
  if (ret != 0) {
    fprintf(stderr, "failed DFU suffix verification (code %d): %s\n", ret, msg);
    exit(1);
  }

  return num_bytes - suffix_length;
}

static size_t load_file(const char *file_name, unsigned char **bytes)
{
  if (file_name == NULL)
    return 0;

  FILE *handle = fopen(file_name, "rb");
  size_t length = file_size(handle, file_name);

  if (handle == NULL) {
    fprintf(stderr, "problem opening file %s\n", file_name);
    exit(1);
  }
  *bytes = malloc(length + 1);
  if (fread(*bytes, 1, length, handle) != length) {
    fprintf(stderr, "problem reading file %s (errno %d)\n", file_name, errno);
    exit(1);
  }

  fclose(handle);

  if (!quiet)
    printf("opened %s, %lu bytes\n", file_name, length);

  return length;
}

static size_t call_verify_suffix(const unsigned char *bytes, size_t length,
                                 struct device_id device_id)
{
  size_t stripped_length = verify_suffix(bytes, length, device_id);

  if (!quiet)
    printf("no problem found with suffix\n");

  return stripped_length;
}

struct inputs read_write_upgrade_inputs(const char *boot_file_name,
                                        const char *data_file_name,
                                        struct device_id device_id)
{
  struct inputs inputs;
  memset(&inputs, 0, sizeof(struct inputs));
  inputs.boot.length = load_file(boot_file_name, &inputs.boot.bytes);
  inputs.data.length = load_file(data_file_name, &inputs.data.bytes);
  inputs.boot.length = call_verify_suffix(inputs.boot.bytes, inputs.boot.length, device_id);
  inputs.data.length = call_verify_suffix(inputs.data.bytes, inputs.data.length, device_id);
  return inputs;
}

struct inputs read_override_spispec_input(const char *spispec_file_name)
{
  struct inputs inputs;
  memset(&inputs, 0, sizeof(struct inputs));
  inputs.spispec.length = load_file(spispec_file_name, &inputs.spispec.bytes);
  return inputs;
}

void cleanup_inputs(struct inputs *inputs)
{
  if (inputs->boot.bytes != NULL) {
    free(inputs->boot.bytes);
    inputs->boot.bytes = NULL;
  }
  if (inputs->data.bytes != NULL) {
    free(inputs->data.bytes);
    inputs->data.bytes = NULL;
  }
  if (inputs->spispec.bytes != NULL) {
    free(inputs->spispec.bytes);
    inputs->spispec.bytes = NULL;
  }
  inputs->boot.length = 0;
  inputs->data.length = 0;
  inputs->spispec.length = 0;
}

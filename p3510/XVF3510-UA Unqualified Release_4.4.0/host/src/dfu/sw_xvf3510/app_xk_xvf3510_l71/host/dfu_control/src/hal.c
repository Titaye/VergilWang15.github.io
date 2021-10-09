// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include <stdio.h>
#include <stdbool.h>
#include "control_host.h"
#include "dfu_commands.h"
#include "util.h"
#include "device_id.h"
#include "sleep.h"
#include "labels.h"
#include "hal.h"

#define KWD_BOOT_COMPLETE 1
#define KWD_BOOT_ERROR 2
#define AP_CONTROL_FLAG 1

extern bool quiet;

#if USE_USB
static int hal_connect_usb(struct device_id device_id)
{
  const int usb_interface_number = 3;
  if (control_init_usb(device_id.vendor, device_id.product, usb_interface_number)
      != CONTROL_SUCCESS) {
    fprintf(stderr, "Error: Control initialisation over USB failed\n");
    return 1;
  }
  if (!quiet) {
    printf("USB connected (vendor ID 0x%04X, product ID 0x%04X)\n",
           device_id.vendor, device_id.product);
  }

  control_version_t version;
  if (control_query_version(&version) != CONTROL_SUCCESS) {
    fprintf(stderr, "Error: Control query version failed\n");
    return 2;
  }
  if (version != CONTROL_VERSION) {
    fprintf(stderr, "Error: Mismatch of the control version between host and device.\
                     Expected 0x%X, received 0x%X\n", CONTROL_VERSION, version);
    return 3;
  }
  if (!quiet)
    printf("control version query successful\n");

  return 0;
}
#endif

#if USE_I2C
static int hal_connect_i2c(struct device_id device_id)
{
  const int shift = 0;
  if (control_init_i2c(device_id.i2c_address << shift) != CONTROL_SUCCESS) {
    fprintf(stderr, "Error: Control initialisation over I2C failed\n");
    return 1;
  }
  if (!quiet)
    printf("I2C connected (slave address 0x%X)\n", device_id.i2c_address);

  control_version_t version;
  if (control_read_command(CONTROL_SPECIAL_RESID, CONTROL_GET_VERSION, &version,
                           sizeof(control_version_t)) != CONTROL_SUCCESS) {
    fprintf(stderr, "Error: Control query version failed\n");
    return 2;
  }
  if (version != CONTROL_VERSION) {
    fprintf(stderr, "Error: Mismatch of the control version between host and device.\
                     Expected 0x%X, received 0x%X\n", CONTROL_VERSION, version);
    return 3;
  }
  if (!quiet)
    printf("control version query successful\n");

  return 0;
}
#endif

int hal_connect(struct device_id device_id)
{
#if USE_USB
  return hal_connect_usb(device_id);
#endif
#if USE_I2C
  return hal_connect_i2c(device_id);
#endif
}

int hal_read_command(enum dfu_command command,
                     unsigned char *payload, size_t num_bytes)
{
  if (!quiet) {
    printf("HAL: read command: %s (%d), %lu bytes\n",
           command_str(command), command, num_bytes);
  }

  if (control_read_command(RESOURCE_ID_DFU, CONTROL_CMD_SET_READ(command),
                           payload, num_bytes) != CONTROL_SUCCESS) {
    fprintf(stderr, "error: control read command did not return success\n");
    return 1;
  }

  return 0;
}

int hal_write_command(enum dfu_command command,
                      const unsigned char *payload, size_t num_bytes)
{
  if (!quiet) {
    printf("HAL: write command: %s (%d), %lu bytes\n",
           command_str(command), command, num_bytes);
  }

  // support empty payload at the HAL level without relying on underlying code
  unsigned char null[1];
  const unsigned char *payload_or_null = payload == NULL ? null : payload;

  if (control_write_command(RESOURCE_ID_DFU, CONTROL_CMD_SET_WRITE(command),
                            payload_or_null, num_bytes) != CONTROL_SUCCESS) {
    fprintf(stderr, "error: control write command did not return success\n");
    return 1;
  }

  return 0;
}

int hal_reboot(void)
{
  if (!quiet)
    printf("HAL: reboot\n");

  if (hal_write_command(DFU_CMD_REBOOT, NULL, 0) != 0)
    return 1;

  return 0;
}

int hal_disconnect(void)
{
#if USE_USB
  if (control_cleanup_usb() != CONTROL_SUCCESS)
    return 1;
#endif
#if USE_I2C
  if (control_cleanup_i2c() != CONTROL_SUCCESS)
    return 1;
#endif

  return 0;
}

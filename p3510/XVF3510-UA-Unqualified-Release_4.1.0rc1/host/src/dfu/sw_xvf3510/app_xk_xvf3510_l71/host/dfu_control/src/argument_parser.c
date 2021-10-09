// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include "argument_parser.h"

bool quiet = false;

void print_usage(FILE *stream)
{
#if USE_USB
  fprintf(stream, "\
usage:      dfu_usb --help\n\
            dfu_usb OPTIONS write_upgrade boot.dfu data.dfu\n\
\n\
OPTIONS:    --quiet\n\
            --vendor-id 0x%04X (default)\n\
            --product-id 0x%04X (default)\n\
            --bcd-device 0x%04X (default)\n\
            --block-size %d (default)\n",
          VENDOR_ID_DEFAULT,
          PRODUCT_ID_DEFAULT,
          BCD_DEVICE_DEFAULT,
          BLOCK_SIZE_DEFAULT);
#endif
#if USE_I2C
  fprintf(stream, "\
usage:      dfu_i2c --help\n\
            dfu_i2c OPTIONS write_upgrade boot.dfu data.dfu\n\
\n\
OPTIONS:    --quiet\n\
            --vendor-id 0x%04X (default)\n\
            --product-id 0x%04X (default)\n\
            --bcd-device 0x%04X (default)\n\
            --i2c-address 0x%02X (default)\n\
            --block-size %d (default)\n",
          VENDOR_ID_DEFAULT,
          PRODUCT_ID_DEFAULT,
          BCD_DEVICE_DEFAULT,
          I2C_ADDRESS_DEFAULT,
          BLOCK_SIZE_DEFAULT);
#endif
}

static const char advanced_usage[] =
#if USE_USB
"\n\
            --skip-boot-image\n\
            --skip-data-image\n\
\n\
advanced:   dfu_usb OPTIONS override_spispec spispec.bin\n\
            dfu_usb OPTIONS detach_and_bus_reset\n\
            dfu_usb OPTIONS reboot\n"
#endif
#if USE_I2C
"\n\
            --skip-boot-image\n\
            --skip-data-image\n\
\n\
advanced:   dfu_i2c OPTIONS override_spispec spispec.bin\n\
            dfu_i2c OPTIONS detach_and_bus_reset\n\
            dfu_i2c OPTIONS reboot\n"
#endif
;

const char *operation_str(int operation)
{
  switch (operation) {
    case WRITE_UPGRADE:        return "write_upgrade";
    case OVERRIDE_SPISPEC:     return "override_spispec";
    case DETACH_AND_BUS_RESET: return "detach_and_bus_reset";
    case REBOOT:               return "reboot";
    default: return "?";
  }
}

int parse_operation(const char *arg)
{
  const int operations[] = {WRITE_UPGRADE, OVERRIDE_SPISPEC,
                            DETACH_AND_BUS_RESET, REBOOT, UNKNOWN};
  for (int i = 0; operations[i] != UNKNOWN; i++) {
    if (strcmp(arg, operation_str(operations[i])) == 0)
      return operations[i];
  }
  return UNKNOWN;
}

struct options parse_arguments(int argc, char **argv)
{
  struct options o = {
    .operation = UNKNOWN,
    .arguments = {NULL, NULL},
    .device_id = {VENDOR_ID_DEFAULT, PRODUCT_ID_DEFAULT,
                  BCD_DEVICE_DEFAULT, I2C_ADDRESS_DEFAULT},
    .block_size = BLOCK_SIZE_DEFAULT,
    .skip_boot_image = false,
    .skip_data_image = false
  };

  if (argc <= 1) {
    print_usage(stderr);
    exit(2);
  }

  int optind = 1;
  for (optind=1; optind<argc; optind++) {
    if ( (strcmp(argv[optind], "--help") == 0 ) || (strcmp(argv[optind], "-h") == 0) ) {
      print_usage(stderr);
      exit(2);
    } else if ( (strcmp(argv[optind], "--help-advanced") == 0 ) || (strcmp(argv[optind], "-a") == 0) ) {
      print_usage(stderr);
      fprintf(stderr, advanced_usage);
      exit(2);
    } else if ( (strcmp(argv[optind], "--quiet") == 0 ) || (strcmp(argv[optind], "-q") == 0) ) {
      quiet = true;
      continue;
    } else if ( (strcmp(argv[optind], "--vendor-id") == 0 ) || (strcmp(argv[optind], "-n") == 0) ) {
      optind++;
      o.device_id.vendor = (uint16_t)strtol(argv[optind], NULL, 0);
      if (o.device_id.vendor == 0 && errno == EINVAL) {
        fprintf(stderr, "error: invalid vendor ID `%s'\n", argv[optind]);
        exit(1);
      }
      if (o.device_id.vendor == 0xFFFF) {
        fprintf(stderr, "\
                error: vendor ID is 0xFFFF, the ignore value for DFU suffix verification\n\
                this is not safe and utility will not proceed\n");
        exit(1);
      }
      continue;
    } else if ( (strcmp(argv[optind], "--product-id") == 0 ) || (strcmp(argv[optind], "-p") == 0) ) {
      optind++;
      o.device_id.product = (uint16_t)strtol(argv[optind], NULL, 0);
      if (o.device_id.product == 0 && errno == EINVAL) {
        fprintf(stderr, "error: invalid product ID `%s'\n", argv[optind]);
        exit(1);
      }
      if (o.device_id.product == 0xFFFF) {
        fprintf(stderr, "\
                error: product ID is 0xFFFF, the ignore value for DFU suffix verification\n\
                this is not safe and utility will not proceed\n");
        exit(1);
      }
      continue;
    } else if ( (strcmp(argv[optind], "--bcd-device") == 0 ) || (strcmp(argv[optind], "-d") == 0) ) {
      optind++;
      o.device_id.bcddevice = (uint16_t)strtol(argv[optind], NULL, 0);
      if (o.device_id.bcddevice == 0 && errno == EINVAL) {
        fprintf(stderr, "error: invalid bcdDevice `%s'\n", argv[optind]);
        exit(1);
      }
      if (o.device_id.bcddevice == 0xFFFF) {
        fprintf(stderr, "\
                warning: bcdDevice is 0xFFFF, the ignore value for DFU suffix verification\n");
        exit(1);
      }
      continue;
    } else if ( (strcmp(argv[optind], "--i2c-address") == 0 ) || (strcmp(argv[optind], "-i") == 0) ) {
      optind++;
      o.device_id.i2c_address = strtol(argv[optind], NULL, 0);
      if (o.device_id.i2c_address == 0 && errno == EINVAL) {
        fprintf(stderr, "error: invalid I2C address `%s'\n", argv[optind]);
      exit(1);
      }
      continue;
    } else if ( (strcmp(argv[optind], "--block-size") == 0 ) || (strcmp(argv[optind], "-b") == 0) ) {
      optind++;
      o.block_size = strtoul(argv[optind], NULL, 0);
      if (o.block_size == 0 && errno == EINVAL) {
        fprintf(stderr, "error: invalid block size `%s'\n", argv[optind]);
        exit(1);
      }
      continue;
    } else if ( (strcmp(argv[optind], "--skip-boot-image") == 0 ) || (strcmp(argv[optind], "-s") == 0) ) {
      o.skip_boot_image = true;
      continue;
    } else if ( (strcmp(argv[optind], "--skip-data-image") == 0 ) || (strcmp(argv[optind], "-t") == 0) ) {
      o.skip_data_image = true;
      continue;
    } else {
      o.operation = parse_operation(argv[optind]);
      switch (o.operation) {
        case WRITE_UPGRADE:
          if (argc != optind + 3) {
            if (argc < optind + 3)
              fprintf(stderr, "error: not enough command line arguments\n");
            else
              fprintf(stderr, "error: too many command line arguments\n");

            print_usage(stderr);
            exit(1);
          }
          o.arguments[0] = argv[optind + 1];
          o.arguments[1] = argv[optind + 2];
          break;

        case OVERRIDE_SPISPEC:
          if (argc != optind + 2) {
            if (argc < optind + 2)
              fprintf(stderr, "error: not enough command line arguments\n");
            else
              fprintf(stderr, "error: too many command line arguments\n");

            print_usage(stderr);
            exit(1);
          }
          o.arguments[0] = argv[optind + 1];
          o.arguments[1] = NULL;
          break;

        case DETACH_AND_BUS_RESET:
          if (argc != optind + 1) {
            print_usage(stderr);
            exit(1);
          }
          o.arguments[0] = NULL;
          o.arguments[1] = NULL;
          break;

        case REBOOT:
          if (argc != optind + 1) {
            print_usage(stderr);
            exit(1);
          }
          o.arguments[0] = NULL;
          o.arguments[1] = NULL;
          break;

        default:
          fprintf(stderr, "unknown operation \"%s\"\n", argv[optind]);
          print_usage(stderr);
          exit(1);
      }

      if (!quiet) {
        printf("options:\n");
        printf("- operation: ");
        switch (o.operation) {
          case WRITE_UPGRADE:
            printf("%s %s %s\n", operation_str(o.operation),
                   o.arguments[0], o.arguments[1]);
            break;

          case OVERRIDE_SPISPEC:
            printf("%s %s\n", operation_str(o.operation), o.arguments[0]);
            break;

          case DETACH_AND_BUS_RESET:
            printf("%s\n", operation_str(o.operation));
            break;

          case REBOOT:
            printf("%s\n", operation_str(o.operation));
            break;

          default:
            printf("%s\n", operation_str(o.operation));
            break;
        }
        printf("- vendor ID 0x%04X, product ID 0x%04X, bcdDevice 0x%04X\n",
               o.device_id.vendor, o.device_id.product, o.device_id.bcddevice);
#if USE_I2C
        printf("- I2C address 0x%02X\n", o.device_id.i2c_address);
#endif
        if (o.operation == WRITE_UPGRADE) {
          printf("- block size %u\n", o.block_size);
          printf("- block size %d\n", o.block_size);
          if (o.skip_boot_image) {
            printf("- skip boot image\n");
          }
          if (o.skip_data_image) {
            printf("- skip data image\n");
          }
        }
      }
      break;
    }
  }
  return o;
}

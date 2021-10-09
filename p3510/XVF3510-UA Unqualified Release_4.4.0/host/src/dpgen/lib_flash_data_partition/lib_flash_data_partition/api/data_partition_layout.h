// Copyright (c) 2020, XMOS Ltd, All rights reserved
#ifndef __data_partition_layout__
#define __data_partition_layout__

#include <stdint.h>
#ifdef __xcore__
#include <quadflash.h>
#endif

#define DP_HARDWARE_BUILD_TAG 0xDAB8DAB8
#define DP_SERIAL_NUMBER_TAG 0xDA5EDA5E
#define DP_IMAGE_TAG 0xDA16DA16

// hardcoded value based on quadflash library
#define QUADFLASH_SPISPEC_SIZE_WORDS 28

// hardcoded value based on Audio Pipelines library
#define FLASH_SERIAL_MAX_CHARACTERS 25

/**
 * Hardware build section
 *
 * Information about hardware available during factory programming
 *
 */
struct dp_hardware_build {
  uint32_t tag;
  uint32_t build; /**< type of build such as prototype or test */
  union {
    uint32_t words[QUADFLASH_SPISPEC_SIZE_WORDS];
#ifdef __xcore__
    fl_QuadDeviceSpec structure[1];
#endif
  } spispec;
  uint32_t checksum;
};

/**
 * Serial number section
 *
 * Runtime programmable non-volatile serial number. Dedicated section so it can
 * be erased prior to programming. Format is typically zero-terminated string.
 *
 */
struct dp_serial_number {
  uint32_t tag;
  uint32_t present; /**< initially set to zero */
  uint8_t serial[FLASH_SERIAL_MAX_CHARACTERS + 1]; /**< add null termination */
};

/**
 * Data image
 *
 * One or more data images follow the previous header sections
 *
 */
struct dp_image_header {
  uint32_t tag;
  uint32_t data_size_words;
  uint32_t compatibility_version;
  uint32_t checksum;
};

#endif

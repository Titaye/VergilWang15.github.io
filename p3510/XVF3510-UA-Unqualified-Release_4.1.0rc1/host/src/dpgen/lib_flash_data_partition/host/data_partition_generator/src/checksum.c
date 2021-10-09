// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include "data_partition_layout.h"
#include "crc.h"
#include "checksum.h"

uint32_t checksum_hardware_build_section(const struct dp_hardware_build *build)
{
  size_t section_words = sizeof(struct dp_hardware_build) / sizeof(int);

  int header_checksum_word_offset =
    offsetof(struct dp_hardware_build, checksum) / sizeof(int);

  unsigned crc = crc_init();
  for (unsigned i = 0; i < section_words; i++) {
    if (i != header_checksum_word_offset)
      crc_step(&crc, ((unsigned*)build)[i]);
  }
  crc_step(&crc, 0); // checksum zeroed out

  return crc_finish(crc);
}

uint32_t checksum_image(const struct dp_image_header *header,
                        const uint8_t *data, size_t data_num_words)
{
  size_t header_words =
    sizeof(struct dp_image_header) / sizeof(int);

  int header_checksum_word_offset =
    offsetof(struct dp_image_header, checksum) / sizeof(int);

  assert(sizeof(struct dp_image_header) % sizeof(int) == 0);
  assert(offsetof(struct dp_image_header, checksum) % sizeof(int) == 0);

  unsigned crc = crc_init();
  for (unsigned i = 0; i < header_words; i++) {
    if (i != header_checksum_word_offset)
      crc_step(&crc, ((const unsigned*)header)[i]);
  }
  for (unsigned i = 0; i < data_num_words; i++) {
    crc_step(&crc, ((const unsigned*)data)[i]);
  }
  crc_step(&crc, 0); // checksum zeroed out and moved after data

  return crc_finish(crc);
}

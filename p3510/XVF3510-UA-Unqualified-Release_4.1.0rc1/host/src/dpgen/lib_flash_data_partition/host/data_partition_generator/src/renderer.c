// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "data_image_layout.h"
#include "inputs.h"
#include "bcd_version.h"
#include "checksum.h"
#include "renderer.h"

extern bool verbose;

size_t pad_to_flash_sector(size_t section_size, unsigned sector_size)
{
  if (section_size % sector_size == 0)
    return section_size;
  else
    return (section_size / sector_size + 1) * sector_size;
}

size_t round_byte_length_to_nearest_word(size_t length)
{
  if (length % sizeof(uint32_t) == 0)
    return length;
  else
    return length + sizeof(uint32_t) - length % sizeof(uint32_t);
}

static size_t count_item_list_size(const struct item *item_list)
{
  size_t length = 0;
  for (const struct item *i = item_list; i != NULL; i = i->next) {
    length += sizeof(struct data_image_tlv_header);
    length += i->num_bytes;
  }
  return round_byte_length_to_nearest_word(length);
}

static size_t estimate_maximum_length(const struct images *images,
                                      unsigned sector_size)
{
  size_t length = 0;

  length += sector_size; // harware build section
  length += sector_size; // serial number section

  if (images->factory.in_use) {
    length += count_item_list_size(images->factory.item_list);
    length += sector_size; // padding and header
  }

  if (images->upgrade.in_use) {
    length += count_item_list_size(images->upgrade.item_list);
    length += sector_size;
  }

  return length;
}

uint32_t render_compatibility_version(const struct version *version)
{
  return BCD_VERSION(version->major, version->minor, version->patch);
}

static void render_hardware_build_section(struct byte_stream *stream,
                                          unsigned hardware_build,
                                          struct spispec spispec,
                                          unsigned sector_size)
{
  struct dp_hardware_build build = {
    .tag = DP_HARDWARE_BUILD_TAG,
    .build = hardware_build,
    .spispec = {0},
    .checksum = 0
  };
  size_t spispec_size = QUADFLASH_SPISPEC_SIZE_WORDS * sizeof(uint32_t);

  if (spispec.num_bytes != spispec_size) {
    fprintf(stderr, "length mismatch of flash specification %s\n\
%lu bytes expected but %lu read\n",
            spispec.file_name, spispec_size, spispec.num_bytes);
    exit(1);
  }

  memcpy(build.spispec.words, spispec.bytes, spispec_size);

  build.checksum = checksum_hardware_build_section(&build);

  size_t length = sizeof(struct dp_hardware_build);
  size_t padded = pad_to_flash_sector(length, sector_size);

  memset(stream->bytes + stream->length, 0, padded);
  memcpy(stream->bytes + stream->length, &build, length);
  stream->length += padded;

  if (verbose)
    printf("hardware build section: %lu bytes padded to %lu\n", length, padded);
}

static void render_serial_number_section(struct byte_stream *stream,
                                         const char *serial_number,
                                         unsigned sector_size)
{
  struct dp_serial_number serial = {
    .tag = DP_SERIAL_NUMBER_TAG,
    .present = serial_number == NULL ? 0 : 1,
    .serial = {(uint8_t)'\0'}
  };

  if (serial_number != NULL) {
    if (strlen(serial_number) > FLASH_SERIAL_MAX_CHARACTERS) {
      fprintf(stderr, "serial number specified longer than "
                      "maximum %d characters plus null termination\n",
                       FLASH_SERIAL_MAX_CHARACTERS);
      exit(1);
    }
    strcpy((char*)serial.serial, serial_number);
  }

  size_t length = sizeof(struct dp_serial_number);
  size_t padded = pad_to_flash_sector(length, sector_size);

  memset(stream->bytes + stream->length, 0, padded);
  memcpy(stream->bytes + stream->length, &serial, length);
  stream->length += padded;

  if (verbose)
    printf("serial number section: %lu bytes padded to %lu\n", length, padded);
}

static struct data_image_tlv_header make_item_header(struct byte_stream *stream,
                                                     const struct item *item)
{
  struct data_image_tlv_header header = {
    .type = (uint8_t)item->type,
    .length = (uint32_t)item->num_bytes
  };
  return header;
}

static struct byte_stream render_item_list(const struct item *item_list)
{
  struct byte_stream stream = {
    .bytes = calloc(count_item_list_size(item_list), 1),
    .length = 0
  };

  for (const struct item *i = item_list; i != NULL; i = i->next) {
    struct data_image_tlv_header header = make_item_header(&stream, i);
    memcpy(stream.bytes + stream.length, &header, sizeof(struct data_image_tlv_header));
    stream.length += sizeof(struct data_image_tlv_header);

    memcpy(stream.bytes + stream.length, i->bytes, i->num_bytes);
    stream.length += i->num_bytes;

    if (verbose) {
      printf("add TLV item %ld bytes\n",
             sizeof(struct data_image_tlv_header) + i->num_bytes);
    }
  }

  stream.length = round_byte_length_to_nearest_word(stream.length);
  if (verbose)
    printf("word aligned item list length %ld bytes\n", stream.length);

  return stream;
}

static void render_image_section(struct byte_stream *stream,
                                 const struct image *image,
                                 bool bad_crc, unsigned sector_size)
{
  struct byte_stream item_list = render_item_list(image->item_list);

  struct dp_image_header header = {
    .tag = DP_IMAGE_TAG,
    .data_size_words = item_list.length / sizeof(uint32_t),
    .compatibility_version =
      render_compatibility_version(&image->compatibility_version),
    .checksum = 0
  };

  header.checksum = checksum_image(&header, item_list.bytes,
                                   item_list.length / sizeof(uint32_t));
  if (bad_crc)
    header.checksum = ~header.checksum;

  size_t length = sizeof(struct dp_image_header) + item_list.length;
  size_t padded = pad_to_flash_sector(length, sector_size);

  memset(stream->bytes + stream->length, 0, padded);
  memcpy(stream->bytes + stream->length, &header, sizeof(struct dp_image_header));
  memcpy(stream->bytes + stream->length + sizeof(struct dp_image_header),
         item_list.bytes, item_list.length);

  stream->length += padded;

  if (verbose)
    printf("image section: %lu bytes padded to %lu\n", length, padded);

  free(item_list.bytes);
}

struct byte_stream render_byte_stream(const struct images *images,
                                      unsigned hardware_build,
                                      const char *serial_number,
                                      struct spispec spispec,
                                      unsigned sector_size,
                                      bool bad_factory_crc,
                                      bool bad_upgrade_crc)
{
  struct byte_stream stream = {
    .bytes = malloc(estimate_maximum_length(images, sector_size)),
    .length = 0
  };

  if (images->factory.in_use) {
    render_hardware_build_section(&stream, hardware_build,
                                  spispec, sector_size);

    render_serial_number_section(&stream, serial_number, sector_size);

    render_image_section(&stream, &images->factory,
                         bad_factory_crc, sector_size);
  }

  if (images->upgrade.in_use) {
    render_image_section(&stream, &images->upgrade,
                         bad_upgrade_crc, sector_size);
  }

  return stream;
}

void cleanup_byte_stream(struct byte_stream stream)
{
  if (stream.bytes != NULL) {
    free(stream.bytes);
  }
}

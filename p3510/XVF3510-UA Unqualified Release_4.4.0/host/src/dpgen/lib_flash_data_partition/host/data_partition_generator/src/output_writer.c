// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include <stdlib.h>
#include <stdio.h>
#include "inputs.h"
#include "renderer.h"
#include "output_writer.h"

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS // fopen safety warnings
#endif

static void write_to_file(const struct byte_stream *stream, const char *file_name)
{
  FILE *handle = fopen(file_name, "wb");
  if (handle == NULL) {
    fprintf(stderr, "problem opening output file \"%s\"\n", file_name);
    exit(1);
  }
  fwrite(stream->bytes, 1, stream->length, handle);
  fclose(handle);
}

void write_output(const struct images *images, unsigned hardware_build,
                  struct spispec spispec, unsigned sector_size,
                  bool bad_factory_crc, bool bad_upgrade_crc,
                  const char *serial_number, const char *out_file_name)
{
  struct byte_stream stream = render_byte_stream(images, hardware_build,
                                                 serial_number, spispec, sector_size,
                                                 bad_factory_crc, bad_upgrade_crc);
  write_to_file(&stream, out_file_name);
  cleanup_byte_stream(stream);
}

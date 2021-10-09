// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <stddef.h>
#include "bcd_version.h"
#include "input_reader.h"
#include "output_writer.h"

bool verbose = false;

// this is a redundant check that we do only because the command line is
// made to look like xflash's, with a version number passed to --upgrade
void check_upgrade_compatibility_version(struct version file,
                                         unsigned argument)
{
  unsigned bcdfile = BCD_VERSION(file.major, file.minor, file.patch);
  if (bcdfile != argument) {
    fprintf(stderr,
"error: command line specified upgrade compatibility version %d/0x%X\n\
but upgrade description file specified %d.%d.%d (%d/0x%X)\n",
      argument, argument, file.major, file.minor, file.patch, bcdfile, bcdfile);
    exit(1);
  }
}

static const char usage[] = "\
usage: data_partition_generator --help\n\
\n\
       data_partition_generator [--verbose] --regular-sector-size N\n\
                                [--hardware-build HARDWARE_BUILD_WORD]\n\
                                [--spi-spec-bin FLASH_SPECIFICATION]\n\
                                [FACTORY] [UPGRADE] -o OUTFILE\n\
\n\
       FACTORY =   --factory FACTORY_FILE\n\
       UPGRADE =   --upgrade COMPATIBILITY_VERSION UPGRADE_FILE\n\
\n\
       HARDWARE_BUILD_WORD is required when factory image is specified\n\
       FLASH_SPECIFICATION is required when factory image is specified\n\
       COMPATIBILITY_VERSION is redundant and provided for consistency with xflash\n\
";

static const char advanced_usage[] = "\
\n\
       --bad-factory-crc    invert CRC for test purposes\n\
       --bad-upgrade-crc    invert CRC for test purposes\n\
       --serial-number STR  set serial number to given string\n\
                            (normally programmed at run time)\n\
";

int main(int argc, char **argv)
{
  uint32_t hardware_build = 0;
  uint32_t regular_sector_size = 0;
  char* factory_file_name = NULL;
  char* upgrade_file_name = NULL; 
  char* output_file_name = NULL;
  char* spispec_file_name = NULL;
  char* serial_number = NULL;
  bool bad_factory_crc = false;
  bool bad_upgrade_crc = false;
  uint32_t upgrade_compatibility_version = 0;

  for (int i=1; i<argc; i++) {
    if ( (strcmp(argv[i], "--hardware-build") == 0 ) || (strcmp(argv[i], "-b") == 0) ) {
      i = i+1;
      hardware_build = strtoul(argv[i], NULL, 0);
      continue;
    } else if ( (strcmp(argv[i], "--serial-number") == 0 ) || (strcmp(argv[i], "-n") == 0) ) {
      i = i+1;
      serial_number = argv[i];
      continue;
    } else if ( (strcmp(argv[i], "--regular-sector-size") == 0 ) || (strcmp(argv[i], "-s") == 0) ) {
      i = i+1;
      regular_sector_size = strtoul(argv[i], NULL, 0);
      continue;
    } else if ( (strcmp(argv[i], "--spi-spec-bin") == 0 ) || (strcmp(argv[i], "-p") == 0) ) {
      i = i+1;
      spispec_file_name = argv[i];
      continue;
    } else if ( (strcmp(argv[i], "--factory") == 0 ) || (strcmp(argv[i], "-f") == 0) ) {
      i = i+1;
      factory_file_name = argv[i];
      continue;
    } else if ( (strcmp(argv[i], "--upgrade") == 0 ) || (strcmp(argv[i], "-u") == 0) ) {
      i = i+1;
      upgrade_compatibility_version = strtoul(argv[i], NULL, 0);
      i = i+1;
      upgrade_file_name = argv[i];
      continue;
    } else if ( (strcmp(argv[i], "--output") == 0 ) || (strcmp(argv[i], "-o") == 0) ) {
      i = i+1;
      output_file_name = argv[i];
      continue;
    } else if ( (strcmp(argv[i], "--bad-factory-crc") == 0 ) || (strcmp(argv[i], "-c") == 0) ) {
      bad_factory_crc = true;
      continue;
    } else if ( (strcmp(argv[i], "--bad-upgrade-crc") == 0 ) || (strcmp(argv[i], "-f") == 0) ) {
      bad_upgrade_crc = true;
      continue;
    } else if ( (strcmp(argv[i], "--help") == 0 ) || (strcmp(argv[i], "-h") == 0) ) {
      fprintf(stderr, usage);
      exit(2);
    } else if ( (strcmp(argv[i], "--help-advanced") == 0 ) || (strcmp(argv[i], "-h") == 0) ) {
      fprintf(stderr, usage);
      fprintf(stderr, advanced_usage);
      exit(2);
    } else if ( (strcmp(argv[i], "--verbose") == 0 ) || (strcmp(argv[i], "-v") == 0) ) {
      verbose = true;
      continue;
    } else {
      fprintf(stderr, "error: invalid argument %s\n", argv[i]);
      exit(1);
    }
  }
  
  if (factory_file_name == NULL && upgrade_file_name == NULL) {
    fprintf(stderr, "error: neither factory nor upgrade specified\n");
    fprintf(stderr, usage);
    exit(1);
  }
  if (regular_sector_size == 0) {
    fprintf(stderr, "error: sector size not specified\n");
    exit(1);
  }
  if (factory_file_name != NULL && hardware_build == 0) {
    fprintf(stderr, "error: factory was specified without hardware build\n");
    exit(1);
  }
  if (factory_file_name != NULL && spispec_file_name == NULL) {
    fprintf(stderr, "error: factory was specified without flash specification\n");
    exit(1);
  }
  if (factory_file_name == NULL && hardware_build != 0) {
    fprintf(stderr, "error (probably): hardware build specified without factory image\n");
    exit(1);
  }
  if (factory_file_name == NULL && spispec_file_name != NULL) {
    fprintf(stderr, "error (probably): flash specification specified without factory image\n");
    exit(1);
  }
  if (upgrade_file_name != NULL && upgrade_compatibility_version == 0) {
    fprintf(stderr, "error: upgrade file specified but not compatibility version\n");
    exit(1);
  }
  if (upgrade_file_name == NULL && upgrade_compatibility_version != 0) {
    fprintf(stderr, "error: upgrade compatibility version specified but not file\n");
    exit(1);
  }

  if (verbose) {
    printf("options: ");
    printf("hardware build 0x%08x, ",
           hardware_build);
    printf("flash specification file name %s, ",
           spispec_file_name);
    printf("factory file name %s, ",
           factory_file_name == NULL ? "-" : factory_file_name);
    printf("upgrade compatibility version %d, ",
           upgrade_compatibility_version);
    printf("upgrade file name %s, ",
           upgrade_file_name == NULL ? "-" : upgrade_file_name);
    printf("out file name %s",
           output_file_name == NULL ? "-" : output_file_name);
    printf("serial number %s",
           serial_number == NULL ? "-" : serial_number);
    printf("%s", bad_factory_crc ? ", bad factory CRC" : NULL);
    printf("%s\n", bad_upgrade_crc ? ", bad upgrade CRC" : NULL);
  }
  struct images images = read_inputs(factory_file_name,
                                     upgrade_file_name);
  if (upgrade_file_name != NULL) {
    check_upgrade_compatibility_version(images.upgrade.compatibility_version,
                                        upgrade_compatibility_version);
  }

  struct spispec spispec = read_flash_specification(spispec_file_name);

  write_output(&images, hardware_build, spispec, regular_sector_size,
               bad_factory_crc, bad_upgrade_crc, serial_number,
               output_file_name);

  if (spispec.valid) {
    free(spispec.bytes);
    spispec.valid = false;
  }

  cleanup_images(&images);

  return 0;
}

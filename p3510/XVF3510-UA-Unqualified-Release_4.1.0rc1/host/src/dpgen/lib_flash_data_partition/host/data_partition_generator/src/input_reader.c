// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <memory.h>
#ifdef _WIN32
#include <errno.h>
#else
#include <sys/errno.h>
#endif
#include "inputs.h"
#include "tokeniser.h"
#include "parser.h"
#include "input_reader.h"

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS // fopen safety warnings
#endif

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

static char *load_file(const char *file_name)
{
  if (file_name == NULL)
    return NULL;

#ifdef _WIN32
  FILE *handle = fopen(file_name, "rb");
#else
  FILE *handle = fopen(file_name, "r");
#endif
  size_t length = file_size(handle, file_name);
  char *string;

  if (handle == NULL) {
    fprintf(stderr, "problem opening file %s\n", file_name);
    exit(1);
  }
  string = malloc(length + 1);
  if (fread(string, 1, length, handle) != length) {
    fprintf(stderr, "problem reading file %s (errno %d)\n", file_name, errno);
    exit(1);
  }
  string[length] = '\0';
  fclose(handle);

  return string;
}

static int parse(struct image *image, const struct token_stream *t)
{
  if (validate_token_stream(t) != 0) {
    fprintf(stderr, "problem with input file\n");
    return 1;
  }

  struct version version = find_compatibility_version(t);
  if (version.major == 0 && version.minor == 0 && version.patch == 0) {
    fprintf(stderr, "not found compatibility version\n");
    return 2;
  }

  image->compatibility_version = version;

  struct item *item_list = construct_item_list(t);
  if (item_list == NULL) {
    fprintf(stderr, "failed to construct list of data items\n");
    return 3;
  }

  image->item_list = item_list;
  return 0;
}

static void build_image(struct image *image,
                        const char *string, const char *file_name)
{
  int ret;

  struct token_stream t;
  t.num_tokens = 0;

  ret = tokenise(&t, string);
  if (ret != 0) {
    fprintf(stderr, "problem parsing file \"%s\" (tokenise)\n", file_name);
    exit(1);
  }

  ret = parse(image, &t);
  if (ret != 0) {
    fprintf(stderr, "problem parsing file \"%s\" (parse)\n", file_name);
    exit(1);
  }

  cleanup_token_stream(&t);
}

struct images read_inputs(const char *factory_file_name,
                          const char *upgrade_file_name)
{
  char *factory_string = load_file(factory_file_name);
  char *upgrade_string = load_file(upgrade_file_name);
  struct images images;

  images.factory.in_use = false;
  images.upgrade.in_use = false;

  if (factory_string != NULL) {
    build_image(&images.factory, factory_string, factory_file_name);
    free(factory_string);
    images.factory.in_use = true;
  }
  if (upgrade_string != NULL) {
    build_image(&images.upgrade, upgrade_string, upgrade_file_name);
    free(upgrade_string);
    images.upgrade.in_use = true;
  }

  return images;
}

struct spispec read_flash_specification(const char *file_name)
{
  if (file_name == NULL) {
    const struct spispec none = {false, "", NULL, 0};
    return none;
  }

  FILE *handle = fopen(file_name, "rb");
  size_t length = file_size(handle, file_name);

  if (handle == NULL) {
    fprintf(stderr, "problem opening file %s\n", file_name);
    exit(1);
  }
  unsigned char *bytes = malloc(length);
  if (fread(bytes, 1, length, handle) != length) {
    fprintf(stderr, "problem reading file %s (errno %d)\n", file_name, errno);
    exit(1);
  }

  fclose(handle);

  struct spispec spec = {true, file_name, bytes, length};
  return spec;
}

void cleanup_images(struct images *images)
{
  if (images->factory.in_use && images->factory.item_list != NULL) {
    cleanup_item_list(images->factory.item_list);
    images->factory.item_list = NULL;
  }
  if (images->upgrade.in_use && images->upgrade.item_list != NULL) {
    cleanup_item_list(images->upgrade.item_list);
    images->upgrade.item_list = NULL;
  }
}

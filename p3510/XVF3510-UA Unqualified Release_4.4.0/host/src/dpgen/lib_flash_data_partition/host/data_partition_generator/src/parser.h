// Copyright (c) 2020, XMOS Ltd, All rights reserved
#ifndef __parser_h__
#define __parser_h__

#include "tokeniser.h"
#include "inputs.h"

#define JSON_COMPATIBILITY_VERSION "compatibility_version"
#define JSON_ITEMS "items"
#define JSON_TYPE "type"
#define JSON_BYTES "bytes"

int validate_token_stream(const struct token_stream *t);

// zero considered invalid version and indicates error
struct version find_compatibility_version(const struct token_stream *t);

struct item *construct_item_list(const struct token_stream *t);

void cleanup_item_list(struct item *item_list);

#endif

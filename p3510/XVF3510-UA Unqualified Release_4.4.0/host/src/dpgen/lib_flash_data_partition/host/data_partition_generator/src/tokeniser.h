// Copyright (c) 2020, XMOS Ltd, All rights reserved
#ifndef __tokeniser_h__
#define __tokeniser_h__

#include <stddef.h>

struct token_stream {
  struct token {
    enum {
      TOKEN_OBJECT = 1,
      TOKEN_ARRAY = 2,
      TOKEN_STRING = 3,
      TOKEN_PRIMITIVE = 4
    } type;
    int str_start;
    int str_len;
    int num_tokens;
  } *tokens;
  int num_tokens;
  char *string;
  size_t string_length;
};

int tokenise(struct token_stream *t, const char *string);

void cleanup_token_stream(struct token_stream *t);

#endif

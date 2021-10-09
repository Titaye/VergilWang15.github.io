// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "jsmn.h"
#include "tokeniser.h"

#define PRINT_JSON_TOKENS 1

extern bool verbose;

static void print_json_tokens(const struct token_stream *t)
{
#if PRINT_JSON_TOKENS
  char *s;
  size_t str_len = 0;
  for (int i = 0; i < t->num_tokens; i++) {
    switch (t->tokens[i].type) {
      case TOKEN_OBJECT:
        printf("    %d: O %d\n", i, t->tokens[i].num_tokens);
        break;

      case TOKEN_ARRAY:
        printf("    %d: A %d\n", i, t->tokens[i].num_tokens);
        break;

      case TOKEN_STRING:
        str_len = t->tokens[i].str_len;  
        s = malloc(str_len + 1);
        memcpy(s, t->string + t->tokens[i].str_start, str_len);
        s[str_len] = '\0';
        printf("    %d: S \"%s\"\n", i, s);
        free(s);
        break;

      case TOKEN_PRIMITIVE:
        str_len = t->tokens[i].str_len;        
        s = malloc(str_len + 1);
        memcpy(s, t->string + t->tokens[i].str_start, str_len);
        s[str_len] = '\0';
        printf("    %d: P \"%s\"\n", i, s);
        free(s);
        break;

      default:
        printf("    %d: ?\n", i);
        break;
    }
  }
#endif
}

int tokenise(struct token_stream *t, const char *string)
{
  size_t string_length = strlen(string);
  int max_tokens = string_length / 4; // heuristic
  int num_tokens = 0;
  jsmntok_t *tokens = malloc(max_tokens * sizeof(jsmntok_t));

  jsmn_parser parser;
  jsmn_init(&parser);

  if (verbose) {
    printf("allocated %ld bytes for jsmn tokens\n",
           max_tokens * sizeof(jsmntok_t));
  }

  num_tokens = jsmn_parse(&parser, string, string_length,
                          tokens, max_tokens);
  if (num_tokens < 0) {
    fprintf(stderr, "jsmn_parse failed (%d)\n", num_tokens);
    return 1;
  }

  t->string_length = string_length;
  t->string = malloc(t->string_length + 1);
  memcpy(t->string, string, t->string_length);
  t->string[t->string_length] = '\0';
  t->num_tokens = num_tokens;
  t->tokens = malloc(num_tokens * sizeof(struct token));

  for (int i = 0; i < num_tokens; i++) {
    t->tokens[i].type = tokens[i].type;
    t->tokens[i].str_start = tokens[i].start;
    t->tokens[i].str_len = tokens[i].end - tokens[i].start;
    t->tokens[i].num_tokens = tokens[i].size;
  }

  if (verbose) {
    printf("parsed into %d tokens\n", num_tokens);
    print_json_tokens(t);
  }

  return 0;
}

void cleanup_token_stream(struct token_stream *t)
{
  if (t->tokens != NULL) {
    free(t->tokens);
    t->tokens = NULL;
  }
  if (t->string != NULL) {
    free(t->string);
    t->string = NULL;
  }
}

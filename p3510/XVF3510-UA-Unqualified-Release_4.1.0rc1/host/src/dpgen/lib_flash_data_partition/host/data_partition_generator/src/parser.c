// Copyright (c) 2020, XMOS Ltd, All rights reserved
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include "data_image_layout.h"
#include "tokeniser.h"
#include "parser.h"

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS // sscanf safety warnings
#endif

extern bool verbose;

void advance_linked_list(struct item **head, struct item **tail)
{
  if (*head == NULL) {
    *head = malloc(sizeof(struct item));
    *tail = *head;
  }
  else {
    (*tail)->next = malloc(sizeof(struct item));
    *tail = (*tail)->next;
  }
  (*tail)->next = NULL;
}

void deallocate_linked_list(struct item *item)
{
  if (item == NULL)
    return;

  if (item->next != NULL) {
    deallocate_linked_list(item->next);
    item->next = NULL;
  }

  free(item);
}

int validate_token_stream(const struct token_stream *t)
{
  const char *strings[4] = {
    JSON_COMPATIBILITY_VERSION, JSON_ITEMS, JSON_TYPE, JSON_BYTES,
  };
  for (int i = 0; i < t->num_tokens; i++) {
    if (t->tokens[i].type == TOKEN_STRING) {
      size_t str_len = t->tokens[i].str_len;
      char *s = malloc(str_len + 1);
      memcpy(s, t->string + t->tokens[i].str_start,
                        str_len);
      s[str_len] = '\0';
      bool match = false;
      int major, minor, patch;
      if (sscanf(s, "%d.%d.%d", &major, &minor, &patch) == 3) {
        match = true;
      }
      if (!match) {
        for (int j = 0; j < 4; j++) {
          if (strcmp(s, strings[j]) == 0) {
            match = true;
            break;
          }
        }
      }
      if (!match) {
        fprintf(stderr, "unrecognised string: %s\n", s);
      }
      free(s);
      if (!match)
        return 1;
    }
  }
  return 0;
}

struct version find_compatibility_version(const struct token_stream *t)
{
  struct version v = {0, 0, 0};
  bool found = false;

  for (int i = 0; i < t->num_tokens; i++) {
    if (t->tokens[i].type == TOKEN_STRING) {
      if (strncmp(t->string + t->tokens[i].str_start,
                  JSON_COMPATIBILITY_VERSION,
                  strlen(JSON_COMPATIBILITY_VERSION)) == 0) {
        if (i < t->num_tokens - 1) {
          if (t->tokens[i + 1].type == TOKEN_STRING) {
            struct version vv;
            char *s = malloc(t->tokens[i + 1].str_len + 1);
            memcpy(s, t->string + t->tokens[i + 1].str_start, t->tokens[i + 1].str_len);
            s[t->tokens[i + 1].str_len] = '\0';
            if (sscanf(s, "%d.%d.%d", &vv.major, &vv.minor, &vv.patch) == 3) {
              found = true;
              v = vv;
            }
            free(s);

            if (found)
              break;
          }
        }
      }
    }
  }

  if (found && verbose)
    printf("compatibility version %d.%d.%d found\n", v.major, v.minor, v.patch);

  return v;
}

struct item *construct_item_list(const struct token_stream *t)
{
  int next_item_token_index = -1;
  int items_remaining = -1;
  struct {
    struct item *head;
    struct item *tail;
  } list = {NULL, NULL};

  for (int j = 0; j < t->num_tokens; j++) {

    // first find the delimiting string that opens the array of data items
    if (t->tokens[j].type == TOKEN_STRING && strncmp(t->string + t->tokens[j].str_start,
        JSON_ITEMS, strlen(JSON_ITEMS)) == 0) {
      if (j + 1 >= t->num_tokens || t->tokens[j + 1].type != TOKEN_ARRAY) {
        fprintf(stderr, "delimiting string \"%s\" not followed by an array\n", JSON_ITEMS);
        return NULL;
      }
      next_item_token_index = j + 2;
      items_remaining = t->tokens[j + 1].num_tokens;
      if (verbose)
        printf("%d fields under \"%s\"\n", items_remaining, JSON_ITEMS);
    }

    // iterate over data items
    if (j == next_item_token_index && items_remaining > 0) {
      advance_linked_list(&list.head, &list.tail);

      if (j + 4 >= t->num_tokens) {
        fprintf(stderr, "incomplete item description\n");
        return NULL;
      }
      if (t->tokens[j].type != TOKEN_OBJECT) {
        fprintf(stderr, "item array element not an object\n");
        return NULL;
      }
      if (t->tokens[j].num_tokens != 2) {
        fprintf(stderr, "item array element not a simple object of num_tokens 2\n");
        return NULL;
      }
      if (t->tokens[j + 1].type != TOKEN_STRING) {
        fprintf(stderr, "first pair of item array element not \"%s\"\n", JSON_TYPE);
        return NULL;
      }
      if (strncmp(t->string + t->tokens[j + 1].str_start,
                  JSON_TYPE, t->tokens[j + 1].str_len) != 0) {
        fprintf(stderr, "first pair of item array element not \"%s\"\n", JSON_TYPE);
        return NULL;
      }
      if (t->tokens[j + 2].type != TOKEN_PRIMITIVE) {
        fprintf(stderr, "first pair of item array element not a number\n");
        return NULL;
      }
      if (t->tokens[j + 3].type != TOKEN_STRING) {
        fprintf(stderr, "second pair of item array element not \"%s\"", JSON_BYTES);
        return NULL;
      }
      if (strncmp(t->string + t->tokens[j + 3].str_start,
                  JSON_BYTES, t->tokens[j + 3].str_len) != 0) {
        fprintf(stderr, "second pair of item array element not \"%s\"\n", JSON_BYTES);
        return NULL;
      }
      if (t->tokens[j + 4].type != TOKEN_ARRAY) {
        fprintf(stderr, "second pair of item array element not an array\n");
        return NULL;
      }

      size_t str_len = t->tokens[j + 2].str_len; 
      char *s = malloc(str_len + 1);
      memcpy(s, t->string + t->tokens[j + 2].str_start, str_len);
      s[str_len] = '\0';
      int num_bytes = t->tokens[j + 4].num_tokens;
      int type = 0;
      sscanf(s, "%d", &type);
      free(s);

      struct item *item = list.tail;
      item->type = type;
      item->num_bytes = num_bytes;
      item->bytes = malloc(num_bytes);

      if (verbose)
        printf("field type %d, %d bytes\n", num_bytes, type);

      for (int k = 0; k < num_bytes; k++) {
        int token_index = j + 5 + k;
        struct token *token = &t->tokens[token_index];
        if (token_index >= t->num_tokens) {
          fprintf(stderr, "not enough item bytes specified\n");
          return NULL;
        }
        if (token->type != TOKEN_PRIMITIVE) {
          fprintf(stderr, "item byte not a number\n");
          return NULL;
        }
        char *endptr = NULL;
        long byte = strtol(t->string + token->str_start, NULL, 0);
        if (byte == 0 && endptr == t->string + token->str_start) {
          fprintf(stderr, "item byte not a number\n");
          return NULL;
        }
        item->bytes[k] = (uint8_t)byte;
      }

      next_item_token_index += num_bytes + 5;
      items_remaining--;
    }
  }

  // terminate
  advance_linked_list(&list.head, &list.tail);
  struct item *item = list.tail;
  item->type = DP_IMAGE_TYPE_TERMINATOR;
  item->num_bytes = 0;
  item->bytes = NULL;

  return list.head;
}

void cleanup_item_list(struct item *item_list)
{
  deallocate_linked_list(item_list);
}

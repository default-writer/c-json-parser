/* SPDX-License-Identifier: BSD-3-Clause */
/*-*-coding:utf-8 -*-
 * Auto updated?
 *   Yes
 * Created:
 *   April 12, 1961 at 09:07:34 PM GMT+3
 * Modified:
 *   February 16, 2026 at 9:09:56 PM GMT+3
 *
 */
/*
    Copyright (C) 2022-2047 default-writer
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its
       contributors may be used to endorse or promote products derived from
       this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "json.h"

extern bool whitespace_lookup[LOOKUP_TABLE_SIZE];
extern const signed char hex_lookup[256];

#define JSON_NULL_LEN 4
#define JSON_TRUE_LEN 4
#define JSON_FALSE_LEN 5
#define HEX_OFFSET 10
#define MIN_PRINTABLE_ASCII 0x20
#define MAX_PRINTABLE_ASCII 0x80
#ifndef _WIN32
#endif

typedef struct {
  char *buf;
  int cap;
  int pos;
} buffer;

#ifndef USE_ALLOC
static json_array_node json_array_node_pool[JSON_VALUE_POOL_SIZE];
static size_t next_array_index = 0;

static json_object_node json_object_node_pool[JSON_VALUE_POOL_SIZE];
static size_t next_object_index = 0;
#endif

static json_value *json_object_get(const json_value *obj, const char *key, size_t len);

static void skip_whitespace(const char **s);
static bool parse_number(const char **s, json_value *v);
static bool parse_string(const char **s, const char *end, json_value *v);
static bool parse_array_value(const char **s, const char *end, json_value *v);
static bool parse_object_value(const char **s, const char *end, json_value *v);
static bool parse_value_build(const char **s, const char *end, json_value *v);

static void print_indent(FILE *out, int indent);
static void print_array_compact(const json_value *v, FILE *out);
static void print_object_compact(const json_value *v, FILE *out);
static void print_value_compact(const json_value *v, FILE *out);
static void print_value(const json_value *v, int indent, FILE *out);

static int buffer_write_indent(buffer *b, int indent);
static int buffer_write_string(buffer *b, const char *s, size_t len);
static int buffer_write_array(buffer *b, const json_value *v);
static int buffer_write_object_indent(buffer *b, const json_value *v, int indent);
static int buffer_write_object(buffer *b, const json_value *v);
static int buffer_write_value_indent(buffer *b, const json_value *v, int indent);
static int buffer_write_value(buffer *b, const json_value *v);

static int buffer_write(buffer *b, const char *data, int len);
static int buffer_putc(buffer *b, char c);

static bool json_array_equal(const json_value *a, const json_value *b);
static bool json_object_equal(const json_value *a, const json_value *b);

static json_array_node *new_array_node();
static json_object_node *new_object_node();
static bool free_array_node(json_array_node *array_node);
static bool free_object_node(json_object_node *object_node);

static INLINE json_value INLINE_ATTRIBUTE *json_object_get(const json_value *obj, const char *key, size_t len) {
  if (!obj || obj->type != J_OBJECT || !key)
    return NULL;
  json_object_node *object_items = obj->u.object.items;
  while (object_items) {
    json_object_node *next = object_items->next;
    if (object_items->item.key.ptr && key && object_items->item.key.len == len && strncmp(object_items->item.key.ptr, key, len) == 0)
      return &object_items->item.value;
    object_items = next;
  }
  return NULL;
}

static INLINE json_array_node *INLINE_ATTRIBUTE new_array_node() {
#ifdef USE_ALLOC
  return (json_array_node *)calloc(1, sizeof(json_array_node));
#else
  if (next_array_index < JSON_VALUE_POOL_SIZE) {
    json_array_node *node = &json_array_node_pool[next_array_index++];
    memset(node, 0, sizeof(json_array_node)); /* Ensure node is zeroed */
    return node;
  }
  return NULL;
#endif
}

static INLINE bool INLINE_ATTRIBUTE free_array_node(json_array_node *array_node) {
#ifdef USE_ALLOC
  free(array_node);
  return true;
#else
  memset(array_node, 0, sizeof(json_array_node));
  return true;
#endif
}

static INLINE json_object_node *INLINE_ATTRIBUTE new_object_node() {
#ifdef USE_ALLOC
  json_object_node *node = (json_object_node *)calloc(1, sizeof(json_object_node));
  return node;
#else
  if (next_object_index < JSON_VALUE_POOL_SIZE) {
    json_object_node *node = &json_object_node_pool[next_object_index++];
    memset(node, 0, sizeof(json_object_node)); /* Ensure node is zeroed */
    return node;
  }
  return NULL;
#endif
}

static INLINE bool INLINE_ATTRIBUTE free_object_node(json_object_node *object_node) {
#ifdef USE_ALLOC
  free(object_node);
  return true;
#else
  memset(object_node, 0, sizeof(json_object_node));
  return true;
#endif
}

static INLINE void INLINE_ATTRIBUTE skip_whitespace(const char **s) {
  while (whitespace_lookup[(unsigned char)**s]) {
    (*s)++;
  }
}

static INLINE bool INLINE_ATTRIBUTE parse_number(const char **s, json_value *v) {
  const char *start_p = *s;
  const char *p = *s;
  if (*p == '-') {
    p++;
  }
  if (*p == '0') {
    p++;
    if (*p >= '1' && *p <= '9') {
      return false;
    }
  } else if (*p >= '1' && *p <= '9') {
    p++;
    while (*p >= '0' && *p <= '9') {
      p++;
    }
  } else {
    return false;
  }
  if (*p == '.') {
    p++;
    if (*p >= '0' && *p <= '9') {
      p++;
      while (*p >= '0' && *p <= '9') {
        p++;
      }
    } else {
      return false;
    }
  }
  if (*p == 'e' || *p == 'E') {
    p++;
    if (*p == '+' || *p == '-') {
      p++;
    }
    if (!(*p >= '0' && *p <= '9')) {
      return false;
    }
    if (*p >= '0' && *p <= '9') {
      p++;
      while (*p >= '0' && *p <= '9') {
        p++;
      }
    } else {
      return false;
    }
  }
  v->u.number.ptr = start_p;
  v->u.number.len = p - start_p;
  *s = p;
  return true;
}

static INLINE bool INLINE_ATTRIBUTE parse_string(const char **s, const char *end, json_value *v) {
  const char *p = *s + 1;
  v->u.string.ptr = p;
  while (true) {
    const size_t len = end - p;
    size_t span = 0;
    const char *s_chk = p - span;
    size_t len_chk = span;
    size_t j = 0;
#if defined(__AVX2__)
    const size_t offset_2 = 32;
    const size_t offset_4 = 64;
    const __m256i quote_vec = _mm256_set1_epi8('\"');
    const __m256i escape_vec = _mm256_set1_epi8('\\');
    size_t i = 0;
    /* Unrolled: process 64 bytes per iteration (2 AVX2 registers) */
    for (; i + offset_4 <= len; i += offset_4) {
      __m256i chunk1 = _mm256_loadu_si256((const __m256i *)(p + i));
      __m256i chunk2 = _mm256_loadu_si256((const __m256i *)(p + i + offset_2));
      __m256i match1 = _mm256_or_si256(_mm256_cmpeq_epi8(chunk1, quote_vec), _mm256_cmpeq_epi8(chunk1, escape_vec));
      __m256i match2 = _mm256_or_si256(_mm256_cmpeq_epi8(chunk2, quote_vec), _mm256_cmpeq_epi8(chunk2, escape_vec));
      int mask1 = _mm256_movemask_epi8(match1);
      int mask2 = _mm256_movemask_epi8(match2);
      if (mask1 != 0) {
        span = i + __builtin_ctz(mask1);
        goto found;
      }
      if (mask2 != 0) {
        span = i + offset_2 + __builtin_ctz(mask2);
        goto found;
      }
    }
    /* Process remaining chunks of 32 bytes */
    for (; i + offset_2 <= len; i += offset_2) {
      __m256i chunk = _mm256_loadu_si256((const __m256i *)(p + i));
      __m256i quote_match = _mm256_cmpeq_epi8(chunk, quote_vec);
      __m256i escape_match = _mm256_cmpeq_epi8(chunk, escape_vec);
      __m256i any_match = _mm256_or_si256(quote_match, escape_match);
      int mask = _mm256_movemask_epi8(any_match);
      if (mask != 0) {
        span = i + __builtin_ctz(mask);
        goto found;
      }
    }
    for (; i < len; i++) {
      if (p[i] == '\"' || p[i] == '\\') {
        span = i;
        goto found;
      }
    }
#if STRING_VALIDATION
    const __m256i limit2 = _mm256_set1_epi8(MIN_PRINTABLE_ASCII);
    const __m256i high_bit2 = _mm256_set1_epi8(MAX_PRINTABLE_ASCII);
    const __m256i limit_shifted2 = _mm256_xor_si256(limit2, high_bit2);
    for (; j + offset_4 <= len_chk; j += offset_4) {
      __m256i chunk1 = _mm256_loadu_si256((const __m256i *)(s_chk + j));
      __m256i chunk2 = _mm256_loadu_si256((const __m256i *)(s_chk + j + offset_2));
      __m256i chunk1_shifted = _mm256_xor_si256(chunk1, high_bit2);
      __m256i chunk2_shifted = _mm256_xor_si256(chunk2, high_bit2);
      __m256i result_mask1 = _mm256_cmpgt_epi8(limit_shifted2, chunk1_shifted);
      __m256i result_mask2 = _mm256_cmpgt_epi8(limit_shifted2, chunk2_shifted);
      if (_mm256_movemask_epi8(result_mask1) != 0 || _mm256_movemask_epi8(result_mask2) != 0) {
        return false;
      }
    }
    for (; j + offset_2 <= len_chk; j += offset_2) {
      __m256i chunk = _mm256_loadu_si256((const __m256i *)(s_chk + j));
      __m256i chunk_shifted = _mm256_xor_si256(chunk, high_bit2);
      __m256i result_mask = _mm256_cmpgt_epi8(limit_shifted2, chunk_shifted);
      if (_mm256_movemask_epi8(result_mask) != 0) {
        return false;
      }
    }
#endif
    span = len;
#elif defined(__SSE2__)
    const size_t offset_1 = 16;
    const size_t offset_2 = 32;
    const size_t offset_3 = 48;
    const size_t offset_4 = 64;
    const __m128i quote_vec = _mm_set1_epi8('\"');
    const __m128i escape_vec = _mm_set1_epi8('\\');
    size_t i = 0;
    /* Unrolled: process 64 bytes per iteration (4 SSE2 registers) */
    for (; i + offset_4 <= len; i += offset_4) {
      __m128i chunk1 = _mm_loadu_si128((const __m128i *)(p + i));
      __m128i chunk2 = _mm_loadu_si128((const __m128i *)(p + i + offset_1));
      __m128i chunk3 = _mm_loadu_si128((const __m128i *)(p + i + offset_2));
      __m128i chunk4 = _mm_loadu_si128((const __m128i *)(p + i + offset_3));
      __m128i match1 = _mm_or_si128(_mm_cmpeq_epi8(chunk1, quote_vec), _mm_cmpeq_epi8(chunk1, escape_vec));
      __m128i match2 = _mm_or_si128(_mm_cmpeq_epi8(chunk2, quote_vec), _mm_cmpeq_epi8(chunk2, escape_vec));
      __m128i match3 = _mm_or_si128(_mm_cmpeq_epi8(chunk3, quote_vec), _mm_cmpeq_epi8(chunk3, escape_vec));
      __m128i match4 = _mm_or_si128(_mm_cmpeq_epi8(chunk4, quote_vec), _mm_cmpeq_epi8(chunk4, escape_vec));
      int mask1 = _mm_movemask_epi8(match1);
      int mask2 = _mm_movemask_epi8(match2);
      int mask3 = _mm_movemask_epi8(match3);
      int mask4 = _mm_movemask_epi8(match4);
      if (mask1 != 0) {
        span = i + __builtin_ctz(mask1);
        goto found;
      }
      if (mask2 != 0) {
        span = i + offset_1 + __builtin_ctz(mask2);
        goto found;
      }
      if (mask3 != 0) {
        span = i + offset_2 + __builtin_ctz(mask3);
        goto found;
      }
      if (mask4 != 0) {
        span = i + offset_3 + __builtin_ctz(mask4);
        goto found;
      }
    }
    /* Process remaining chunks of 16 bytes */
    for (; i + offset_1 <= len; i += offset_1) {
      __m128i chunk = _mm_loadu_si128((const __m128i *)(p + i));
      __m128i quote_match = _mm_cmpeq_epi8(chunk, quote_vec);
      __m128i escape_match = _mm_cmpeq_epi8(chunk, escape_vec);
      __m128i any_match = _mm_or_si128(quote_match, escape_match);
      int mask = _mm_movemask_epi8(any_match);
      if (mask != 0) {
        span = i + __builtin_ctz(mask);
        goto found;
      }
    }
    for (; i < len; i++) {
      if (p[i] == '\"' || p[i] == '\\') {
        span = i;
        goto found;
      }
    }
#if STRING_VALIDATION
    const __m128i limit2 = _mm_set1_epi8(MIN_PRINTABLE_ASCII);
    const __m128i high_bit2 = _mm_set1_epi8(MAX_PRINTABLE_ASCII);
    const __m128i limit_shifted2 = _mm_xor_si128(limit2, high_bit2);
    for (; j + offset_4 <= len_chk; j += offset_4) {
      __m128i chunk1 = _mm_loadu_si128((const __m128i *)(s_chk + j));
      __m128i chunk2 = _mm_loadu_si128((const __m128i *)(s_chk + j + offset_1));
      __m128i chunk3 = _mm_loadu_si128((const __m128i *)(s_chk + j + offset_2));
      __m128i chunk4 = _mm_loadu_si128((const __m128i *)(s_chk + j + offset_3));
      __m128i chunk1_shifted = _mm_xor_si128(chunk1, high_bit2);
      __m128i chunk2_shifted = _mm_xor_si128(chunk2, high_bit2);
      __m128i chunk3_shifted = _mm_xor_si128(chunk3, high_bit2);
      __m128i chunk4_shifted = _mm_xor_si128(chunk4, high_bit2);
      __m128i result_mask1 = _mm_cmplt_epi8(chunk1_shifted, limit_shifted2);
      __m128i result_mask2 = _mm_cmplt_epi8(chunk2_shifted, limit_shifted2);
      __m128i result_mask3 = _mm_cmplt_epi8(chunk3_shifted, limit_shifted2);
      __m128i result_mask4 = _mm_cmplt_epi8(chunk4_shifted, limit_shifted2);
      if (_mm_movemask_epi8(result_mask1) != 0 || _mm_movemask_epi8(result_mask2) != 0 ||
          _mm_movemask_epi8(result_mask3) != 0 || _mm_movemask_epi8(result_mask4) != 0) {
        return false;
      }
    }
    for (; j + offset_1 <= len_chk; j += offset_1) {
      __m128i chunk = _mm_loadu_si128((const __m128i *)(s_chk + j));
      __m128i chunk_shifted = _mm_xor_si128(chunk, high_bit2);
      __m128i result_mask = _mm_cmplt_epi8(chunk_shifted, limit_shifted2);
      if (_mm_movemask_epi8(result_mask) != 0) {
        return false;
      }
    }
#endif
    span = len;
#else
    /* non-SSE2 fallback: simple byte scan */
    size_t i = 0;
    for (; i < len; i++) {
      if (p[i] == '\"' || p[i] == '\\') {
        span = i;
        goto found;
      }
    }
    span = len;
#endif

    for (; j < len_chk; j++) {
      if ((unsigned char)s_chk[j] < MIN_PRINTABLE_ASCII) {
        return false;
      }
    }

    /* scalar tail */
    for (; i < len; i++) {
      if ((unsigned char)p[i] < MIN_PRINTABLE_ASCII || (unsigned char)p[i] >= MAX_PRINTABLE_ASCII) {
        return false;
      }
      if (p[i] == '"' || p[i] == '\\') {
        span = i;
        goto found;
      }
    }

    return false;
  found:
    p += span;
    if (p == end)
      return false;
    if (*p == '"') {
      /* Inline SIMD validation for the parsed string span */
      const char *chk = v->u.string.ptr;
      size_t chk_len = (size_t)(p - chk);
      size_t ii = 0;
#if defined(__AVX2__)
      const size_t offset_4 = 64;
      const size_t offset_avx = 32;
      const __m256i limit_avx = _mm256_set1_epi8((char)MIN_PRINTABLE_ASCII);
      const __m256i high_bit_avx = _mm256_set1_epi8((char)MAX_PRINTABLE_ASCII);
      const __m256i limit_shifted_avx = _mm256_xor_si256(limit_avx, high_bit_avx);
      for (; ii + offset_4 <= chk_len; ii += offset_4) {
        __m256i c1 = _mm256_loadu_si256((const __m256i *)(chk + ii));
        __m256i c2 = _mm256_loadu_si256((const __m256i *)(chk + ii + offset_avx));
        __m256i c1s = _mm256_xor_si256(c1, high_bit_avx);
        __m256i c2s = _mm256_xor_si256(c2, high_bit_avx);
        __m256i rm1 = _mm256_cmpgt_epi8(limit_shifted_avx, c1s);
        __m256i rm2 = _mm256_cmpgt_epi8(limit_shifted_avx, c2s);
        if (_mm256_movemask_epi8(rm1) != 0 || _mm256_movemask_epi8(rm2) != 0)
          return false;
      }
      for (; ii + offset_avx <= chk_len; ii += offset_avx) {
        __m256i c = _mm256_loadu_si256((const __m256i *)(chk + ii));
        __m256i cs = _mm256_xor_si256(c, high_bit_avx);
        __m256i rm = _mm256_cmpgt_epi8(limit_shifted_avx, cs);
        if (_mm256_movemask_epi8(rm) != 0)
          return false;
      }
#elif defined(__SSE2__)
      const size_t offset_sse = 16;
      const size_t offset_2 = 16;
      const size_t offset_3 = 32;
      const size_t offset_4 = 64;
      const __m128i limit_sse = _mm_set1_epi8((char)MIN_PRINTABLE_ASCII);
      const __m128i high_bit_sse = _mm_set1_epi8((char)MAX_PRINTABLE_ASCII);
      const __m128i limit_shifted_sse = _mm_xor_si128(limit_sse, high_bit_sse);
      for (; ii + offset_4 <= chk_len; ii += offset_4) {
        __m128i c1 = _mm_loadu_si128((const __m128i *)(chk + ii));
        __m128i c2 = _mm_loadu_si128((const __m128i *)(chk + ii + offset_2));
        __m128i c3 = _mm_loadu_si128((const __m128i *)(chk + ii + offset_3));
        __m128i c4 = _mm_loadu_si128((const __m128i *)(chk + ii + offset_4));
        __m128i c1s = _mm_xor_si128(c1, high_bit_sse);
        __m128i c2s = _mm_xor_si128(c2, high_bit_sse);
        __m128i c3s = _mm_xor_si128(c3, high_bit_sse);
        __m128i c4s = _mm_xor_si128(c4, high_bit_sse);
        __m128i rm1 = _mm_cmplt_epi8(c1s, limit_shifted_sse);
        __m128i rm2 = _mm_cmplt_epi8(c2s, limit_shifted_sse);
        __m128i rm3 = _mm_cmplt_epi8(c3s, limit_shifted_sse);
        __m128i rm4 = _mm_cmplt_epi8(c4s, limit_shifted_sse);
        if (_mm_movemask_epi8(rm1) != 0 || _mm_movemask_epi8(rm2) != 0 || _mm_movemask_epi8(rm3) != 0 || _mm_movemask_epi8(rm4) != 0)
          return false;
      }
      for (; ii + offset_sse <= chk_len; ii += offset_sse) {
        __m128i c = _mm_loadu_si128((const __m128i *)(chk + ii));
        __m128i cs = _mm_xor_si128(c, high_bit_sse);
        __m128i rm = _mm_cmplt_epi8(cs, limit_shifted_sse);
        if (_mm_movemask_epi8(rm) != 0)
          return false;
      }
      for (; ii < chk_len; ii++) {
        if ((unsigned char)chk[ii] < MIN_PRINTABLE_ASCII)
          return false;
      }
#else
      for (; ii < chk_len; ii++) {
        if ((unsigned char)chk[ii] < MIN_PRINTABLE_ASCII)
          return false;
      }
#endif
      /* scalar tail for any remaining bytes not covered by SIMD loops */
      v->u.string.len = p - *s - 1;
      *s = p + 1;
      return true;
    }
    if (*p != '\\') {
      return false;
    }
    if (++p == end)
      return false;
    switch (*p) {
    case '\"':
    case '\\':
    case '/':
    case 'b':
    case 'f':
    case 'n':
    case 'r':
    case 't':
      if (++p == end)
        return false;
      break;
    case 'u':
      if (++p == end)
        return false;
      if (p + 3 >= end)
        return false;
      if (hex_lookup[(unsigned char)p[0]] < 0 || hex_lookup[(unsigned char)p[1]] < 0 || hex_lookup[(unsigned char)p[2]] < 0 || hex_lookup[(unsigned char)p[3]] < 0)
        return false;
      if (p + 4 < end && hex_lookup[(unsigned char)p[4]] >= 0)
        return false;
      p += 4;
      break;
    default:
      return false;
    }
  }
}

static INLINE bool INLINE_ATTRIBUTE parse_array_value(const char **s, const char *end, json_value *v) {
  while (true) {
    skip_whitespace(s);
    if (*s == end)
      return false;
    json_array_node *array_node = new_array_node();
    if (array_node == NULL) {
      return false;
    }
    do {
      if (v->u.array.items == NULL) {
        v->u.array.items = array_node;
        break;
      }
      if (v->u.array.last == NULL) {
        v->u.array.last = array_node;
        v->u.array.items->next = array_node;
        break;
      }
      json_array_node *last = v->u.array.last;
      v->u.array.last = array_node;
      last->next = array_node;
    } while (0);
    if (!parse_value_build(s, end, &array_node->item)) {
      free_array_node(array_node);
      v->u.array.items = NULL;
      return false;
    }
    skip_whitespace(s);
    if (*s == end)
      return false;
    if (**s == ',') {
      (*s)++;
      continue;
    }
    if (**s == ']') {
      (*s)++;
      return true;
    }
    return false;
  }
}

static INLINE bool INLINE_ATTRIBUTE parse_object_value(const char **s, const char *end, json_value *v) {
  while (true) {
    skip_whitespace(s);
    if (*s == end)
      return false;
    if (**s != '\"') {
      return false;
    }
    json_value key;
    key.type = J_STRING;
    if (!parse_string(s, end, &key)) {
      return false;
    }
    skip_whitespace(s);
    if (*s == end)
      return false;
    if (**s != ':') {
      return false;
    }
    (*s)++;
    skip_whitespace(s);
    if (*s == end)
      return false;
    json_object_node *object_node = NULL;
    json_object_node *object_items = v->u.object.items;
    while (object_items) {
      json_object_node *next = object_items->next;
      if (object_items->item.key.ptr && object_items->item.key.len == key.u.string.len && strncmp(object_items->item.key.ptr, key.u.string.ptr, key.u.string.len) == 0) {
        break;
      }
      object_items = next;
    }
    if (object_items == NULL) {
      object_node = new_object_node();
      if (object_node == NULL) {
        return false;
      }
      object_node->item.key.ptr = key.u.string.ptr;
      object_node->item.key.len = key.u.string.len;
      do {
        if (v->u.object.items == NULL) {
          v->u.object.items = object_node;
          break;
        }
        if (v->u.object.last == NULL) {
          v->u.object.last = object_node;
          v->u.object.items->next = object_node;
          break;
        }
        json_object_node *last = v->u.object.last;
        v->u.object.last = object_node;
        last->next = object_node;
      } while (0);
    } else {
      object_node = object_items;
    }
    if (!parse_value_build(s, end, &object_node->item.value)) {
      free_object_node(object_node);
      v->u.object.items = NULL;
      return false;
    }
    skip_whitespace(s);
    if (*s == end)
      return false;
    if (**s == ',') {
      (*s)++;
      continue;
    }
    if (**s == '}') {
      (*s)++;
      return true;
    }
    return false;
  }
}

static bool parse_value_build(const char **s, const char *end, json_value *v) {
  if (**s == '{') {
    v->type = J_OBJECT;
    v->u.object.items = NULL;
    v->u.object.last = NULL;
    (*s)++;
    skip_whitespace(s);
    if (*s == end)
      return false;
    if (**s == '}') {
      (*s)++;
      return true;
    }
    return parse_object_value(s, end, v);
  }
  if (**s == '[') {
    v->type = J_ARRAY;
    v->u.array.items = NULL;
    v->u.array.last = NULL;
    (*s)++;
    if (*s == end)
      return false;
    skip_whitespace(s);
    if (*s == end)
      return false;
    if (**s == ']') {
      (*s)++;
      return true;
    }
    return parse_array_value(s, end, v);
  }
  if (**s == '\"') {
    v->type = J_STRING;
    return parse_string(s, end, v);
  }
  if (**s == 'n' && *(*s + 1) == 'u' && *(*s + 2) == 'l' && *(*s + 3) == 'l') {
    v->type = J_NULL;
    v->u.string.ptr = *s;
    v->u.string.len = 4;
    *s += 4;
    return true;
  }
  if (**s == 't' && *(*s + 1) == 'r' && *(*s + 2) == 'u' && *(*s + 3) == 'e') {
    v->type = J_BOOLEAN;
    v->u.boolean.ptr = *s;
    v->u.boolean.len = JSON_TRUE_LEN;
    *s += JSON_TRUE_LEN;
    return true;
  }
  if (**s == 'f' && *(*s + 1) == 'a' && *(*s + 2) == 'l' && *(*s + 3) == 's' && *(*s + 4) == 'e') {
    v->type = J_BOOLEAN;
    v->u.boolean.ptr = *s;
    v->u.boolean.len = JSON_FALSE_LEN;
    *s += JSON_FALSE_LEN;
    return true;
  }
  if (parse_number(s, v)) {
    v->type = J_NUMBER;
    return true;
  }
  return false;
}

/* --- pretty-print helpers --- */

static INLINE void INLINE_ATTRIBUTE print_indent(FILE *out, int indent) {
  int i;
  for (i = 0; i < indent; ++i)
    fputs("    ", out);
}

static INLINE void INLINE_ATTRIBUTE print_array_compact(const json_value *v, FILE *out) {
  fputc('[', out);
  json_array_node *array_items = v->u.array.items;
  while (array_items) {
    json_array_node *next = array_items->next;
    print_value_compact(&array_items->item, out);
    if (next)
      fputs(", ", out);
    array_items = next;
  }
  fputc(']', out);
}

static INLINE void INLINE_ATTRIBUTE print_object_compact(const json_value *v, FILE *out) {
  fputc('{', out);
  json_object_node *object_items = v->u.object.items;
  while (object_items) {
    json_object_node *next = object_items->next;
    fprintf(out, "\"%.*s\"", (int)object_items->item.key.len, object_items->item.key.ptr);
    fputs(": ", out);
    print_value_compact(&object_items->item.value, out);
    if (next)
      fputs(", ", out);
    object_items = next;
  }
  fputc('}', out);
}

static void print_value_compact(const json_value *v, FILE *out) {
  switch (v->type) {
  case J_NULL:
    fputs("null", out);
    break;
  case J_BOOLEAN:
    fprintf(out, "%.*s", (int)v->u.boolean.len, v->u.boolean.ptr);
    break;
  case J_NUMBER:
    fprintf(out, "%.*s", (int)v->u.number.len, v->u.number.ptr);
    break;
  case J_STRING:
    fprintf(out, "\"%.*s\"", (int)v->u.string.len, v->u.string.ptr);
    break;
  case J_ARRAY:
    print_array_compact(v, out);
    break;
  case J_OBJECT:
    print_object_compact(v, out);
    break;
  }
}

static void print_value(const json_value *v, int indent, FILE *out) {
  switch (v->type) {
  case J_NULL:
    fputs("null", out);
    break;
  case J_BOOLEAN:
    fprintf(out, "%.*s", (int)v->u.boolean.len, v->u.boolean.ptr);
    break;
  case J_NUMBER:
    fprintf(out, "%.*s", (int)v->u.number.len, v->u.number.ptr);
    break;
  case J_STRING:
    fprintf(out, "\"%.*s\"", (int)v->u.string.len, v->u.string.ptr);
    break;
  case J_ARRAY:
    print_array_compact(v, out);
    break;
  case J_OBJECT:
    fputs("{\n", out);
    json_object_node *object_items = v->u.object.items;
    while (object_items) {
      json_object_node *next = object_items->next;
      print_indent(out, indent + 1);
      fprintf(out, "\"%.*s\"", (int)object_items->item.key.len, object_items->item.key.ptr);
      fputs(": ", out);
      print_value(&object_items->item.value, indent + 1, out);
      if (next)
        fputs(", ", out);
      object_items = next;
    }
    fputc('\n', out);
    print_indent(out, indent);
    fputc('}', out);
    break;
  }
}

static INLINE int INLINE_ATTRIBUTE buffer_write_indent(buffer *b, int indent) {
  int i;
  for (i = 0; i < indent; ++i) {
    if (buffer_write(b, "    ", 4) < 0)
      return -1;
  }
  return 0;
}

static INLINE int INLINE_ATTRIBUTE buffer_write_string(buffer *b, const char *s, size_t len) {
  if (buffer_putc(b, '\"') < 0)
    return -1;
  if (buffer_write(b, s, len) < 0)
    return -1;
  if (buffer_putc(b, '\"') < 0)
    return -1;
  return 0;
}

static INLINE int INLINE_ATTRIBUTE buffer_write_array(buffer *b, const json_value *v) {
  if (buffer_putc(b, '[') < 0)
    return -1;
  json_array_node *array_items = v->u.array.items;
  while (array_items) {
    json_array_node *next = array_items->next;
    if (buffer_write_value(b, &array_items->item) < 0)
      return -1;
    if (next) {
      if (buffer_write(b, ", ", 2) < 0)
        return -1;
    }
    array_items = next;
  }
  if (buffer_putc(b, ']') < 0)
    return -1;
  return 0;
}

static INLINE int INLINE_ATTRIBUTE buffer_write_object_indent(buffer *b, const json_value *v, int indent) {
  json_object_node *object_items = v->u.object.items;
  if (object_items == NULL) {
    if (buffer_write(b, "{\n", 2) < 0)
      return -1;
    if (buffer_write_indent(b, indent) < 0)
      return -1;
    if (buffer_putc(b, '}') < 0)
      return -1;
    return 0;
  }
  if (buffer_write(b, "{\n", 2) < 0)
    return -1;
  while (object_items) {
    json_object_node *next = object_items->next;
    if (buffer_write_indent(b, indent + 1) < 0)
      return -1;
    json_object *ent = &object_items->item;
    if (buffer_write_string(b, ent->key.ptr, ent->key.len) < 0)
      return -1;
    if (buffer_write(b, ": ", 2) < 0)
      return -1;
    if (buffer_write_value_indent(b, &ent->value, indent + 1) < 0)
      return -1;
    if (next) {
      if (buffer_write(b, ",\n", 2) < 0)
        return -1;
    }
    object_items = next;
  }
  if (buffer_putc(b, '\n') < 0)
    return -1;
  if (buffer_write_indent(b, indent) < 0)
    return -1;
  if (buffer_putc(b, '}') < 0)
    return -1;
  return 0;
}

static INLINE int INLINE_ATTRIBUTE buffer_write_object(buffer *b, const json_value *v) {
  json_object_node *object_items = v->u.object.items;
  if (object_items == NULL)
    return buffer_write(b, "{}", 2);
  if (buffer_putc(b, '{') < 0)
    return -1;
  while (object_items) {
    json_object_node *next = object_items->next;
    json_object *ent = &object_items->item;
    if (buffer_write_string(b, ent->key.ptr, ent->key.len) < 0)
      return -1;
    if (buffer_write(b, ": ", 2) < 0)
      return -1;
    if (buffer_write_value(b, &ent->value) < 0)
      return -1;
    if (next) {
      if (buffer_write(b, ", ", 2) < 0)
        return -1;
    }
    object_items = next;
  }
  if (buffer_putc(b, '}') < 0)
    return -1;
  return 0;
}

static INLINE int buffer_write_value_indent(buffer *b, const json_value *v, int indent) {
  switch (v->type) {
  case J_NULL:
    return buffer_write(b, "null", 4);
  case J_BOOLEAN:
    return buffer_write(b, v->u.boolean.ptr, v->u.boolean.len);
  case J_NUMBER:
    return buffer_write(b, v->u.number.ptr, (int)v->u.number.len);
  case J_STRING:
    return buffer_write_string(b, v->u.string.ptr, v->u.string.len);
  case J_ARRAY:
    return buffer_write_array(b, v);
  case J_OBJECT:
    return buffer_write_object_indent(b, v, indent);
  }
  return -1;
}

static INLINE int buffer_write_value(buffer *b, const json_value *v) {
  switch (v->type) {
  case J_NULL:
    return buffer_write(b, "null", 4);
  case J_BOOLEAN:
    return buffer_write(b, v->u.boolean.ptr, v->u.boolean.len);
  case J_NUMBER:
    return buffer_write(b, v->u.number.ptr, (int)v->u.number.len);
  case J_STRING:
    return buffer_write_string(b, v->u.string.ptr, v->u.string.len);
  case J_ARRAY:
    return buffer_write_array(b, v);
  case J_OBJECT:
    return buffer_write_object(b, v);
  }
  return -1;
}

/* --- buffer helpers --- */

static INLINE int INLINE_ATTRIBUTE buffer_write(buffer *b, const char *data, int len) {
  if (len <= 0)
    return 0;
  if (b->pos + len + 1 >= b->cap) {
    int new_cap = b->cap ? b->cap : 1;
    while (b->pos + len + 1 >= new_cap) {
      new_cap = new_cap * 2;
    }
    char *new_buf = (char *)realloc(b->buf, new_cap);
    if (!new_buf)
      return -1;
    b->buf = new_buf;
    b->cap = new_cap;
  }
  memcpy(b->buf + b->pos, data, len);
  b->pos += len;
  return 0;
}

static INLINE int INLINE_ATTRIBUTE buffer_putc(buffer *b, char c) {
  if (b->pos + 1 >= b->cap) {
    int new_cap = b->cap * 2;
    char *new_buf = (char *)realloc(b->buf, new_cap);
    if (!new_buf)
      return -1;
    b->buf = new_buf;
    b->cap = new_cap;
  }
  b->buf[b->pos++] = c;
  return 0;
}

/* --- json helpers --- */

INLINE char *INLINE_ATTRIBUTE json_stringify(const json_value *v) {
  if (!v)
    return NULL;
  buffer b;
  b.cap = MAX_BUFFER_SIZE;
  b.pos = 0;
  b.buf = (char *)calloc(1, (size_t)b.cap);
  if (!b.buf)
    return NULL;
  if (buffer_write_value_indent(&b, v, 0) < 0) {
    free(b.buf);
    return NULL;
  }
  char *final_buf = (char *)realloc(b.buf, (size_t)b.pos + 1);
  if (!final_buf) {
    free(b.buf);
    return NULL;
  }
  final_buf[b.pos] = '\0';
  return final_buf;
}

static INLINE bool INLINE_ATTRIBUTE json_array_equal(const json_value *a, const json_value *b) {
  json_array_node *a_node;
  json_array_node *b_node;
  if (a->type != J_ARRAY || b->type != J_ARRAY)
    return false;
  size_t a_len = 0;
  for (a_node = a->u.array.items; a_node; a_node = a_node->next) {
    a_len++;
  }
  size_t b_len = 0;
  for (b_node = b->u.array.items; b_node; b_node = b_node->next) {
    b_len++;
  }
  if (a_len != b_len) {
    return false;
  }
  json_array_node *a_array_items = a->u.array.items;
  json_array_node *b_array_items = b->u.array.items;
  while (a_array_items && b_array_items) {
    if (!json_equal(&a_array_items->item, &b_array_items->item))
      return false;
    a_array_items = a_array_items->next;
    b_array_items = b_array_items->next;
  }
  return true;
}

static INLINE bool INLINE_ATTRIBUTE json_object_equal(const json_value *a, const json_value *b) {
  json_object_node *a_node;
  json_object_node *b_node;
  json_object *e;
  json_value *b_val;
  if (a->type != J_OBJECT || b->type != J_OBJECT)
    return false;
  size_t a_len = 0;
  for (a_node = a->u.object.items; a_node; a_node = a_node->next) {
    a_len++;
  }
  size_t b_len = 0;
  for (b_node = b->u.object.items; b_node; b_node = b_node->next) {
    b_len++;
  }
  if (a_len != b_len) {
    return false;
  }
  for (a_node = a->u.object.items; a_node; a_node = a_node->next) {
    e = &a_node->item;
    b_val = json_object_get(b, e->key.ptr, e->key.len);
    if (!b_val || !json_equal(&e->value, b_val)) {
      return false;
    }
  }
  return true;
}

/* --- public API --- */

INLINE json_error INLINE_ATTRIBUTE json_validate(const char **s, size_t len) {
  if (**s == '\0')
    return E_NO_DATA;
  if (**s != '{' && **s != '[') {
    return E_INVALID_JSON;
  }
  const char *end = *s + len;
  json_value *stack[JSON_STACK_SIZE];
  json_value v;
  memset(&v, 0, sizeof(json_value));
  int top = -1;
  json_value *current = &v;
  while (true) {
    if (*s == end)
      break;
    skip_whitespace(s);
    if (*s == end)
      break;
    if (current) {
      do {
        if (**s == '{') {
          current->type = J_OBJECT;
          current->u.object.items = NULL;
          current->u.object.last = NULL;
          if (++top >= JSON_STACK_SIZE)
            return E_STACK_OVERFLOW_OBJECT;
          (*s)++;
          stack[top] = current;
          current = NULL;
          break;
        }
        if (**s == '[') {
          current->type = J_ARRAY;
          current->u.array.items = NULL;
          current->u.array.last = NULL;
          (*s)++;
          if (++top >= JSON_STACK_SIZE)
            return E_STACK_OVERFLOW_ARRAY;
          stack[top] = current;
          current = NULL;
          break;
        }
        if (**s == '\"') {
          current->type = J_STRING;
          if (parse_string(s, end, current)) {
            current = NULL;
            break;
          }
          return E_EXPECTED_STRING;
        }
        if (**s == 't') {
          if (*(*s + 1) == 'r' && *(*s + 2) == 'u' && *(*s + 3) == 'e') {
            current->type = J_BOOLEAN;
            current->u.boolean.ptr = *s;
            current->u.boolean.len = 4;
            *s += 4;
            current = NULL;
            break;
          }
          return E_EXPECTED_BOOLEAN;
        }
        if (**s == 'f') {
          if (*(*s + 1) == 'a' && *(*s + 2) == 'l' && *(*s + 3) == 's' && *(*s + 4) == 'e') {
            current->type = J_BOOLEAN;
            current->u.boolean.ptr = *s;
            current->u.boolean.len = JSON_FALSE_LEN;
            *s += JSON_FALSE_LEN;
            current = NULL;
            break;
          }
          return E_EXPECTED_BOOLEAN;
        }
        if (**s == 'n') {
          if (*(*s + 1) == 'u' && *(*s + 2) == 'l' && *(*s + 3) == 'l') {
            current->type = J_NULL;
            *s += 4;
            current = NULL;
            break;
          }
          return E_EXPECTED_NULL;
        }
        if (!parse_number(s, current)) {
          return E_MAILFORMED_JSON;
        }
        current->type = J_NUMBER;
        current = NULL;
        break;
      } while (0);
      if (!current) {
        continue;
      }
    }
    if (top == -1) {
      break;
    }
    current = stack[top];
    if (current->type == J_OBJECT) {
      if (**s == '}') {
        top--;
        (*s)++;
        if (*s == end) {
          break;
        }
        current = NULL;
        continue;
      }
      if (current->u.object.items != NULL) {
        if (**s == ',') {
          (*s)++;
          skip_whitespace(s);
          if (*s == end || **s == '}') {
            return E_EXPECTED_OBJECT_ELEMENT;
          }
        } else {
          return E_EXPECTED_OBJECT;
        }
      }
      if (**s != '\"') {
        return E_OBJECT_KEY;
      }
      json_value key;
      if (!parse_string(s, end, &key))
        return E_EXPECTED_OBJECT_KEY;
      skip_whitespace(s);
      if (**s != ':') {
        return E_OBJECT_VALUE;
      }
      (*s)++;
      skip_whitespace(s);
      if (*s == end) {
        return E_EXPECTED_OBJECT_VALUE;
      }
      json_object_node *node = new_object_node();
      if (!node)
        return E_STACK_OVERFLOW_OBJECT;
      node->item.key = key.u.string;
      if (current->u.object.items == NULL) {
        current->u.object.items = node;
      } else {
        current->u.object.last->next = node;
      }
      current->u.object.last = node;
      current = &node->item.value;
      continue;
    }
    if (current->type == J_ARRAY) {
      if (**s == ']') {
        top--;
        (*s)++;
        if (*s == end) {
          break;
        }
        current = NULL;
        continue;
      }
      if (current->u.array.items != NULL) {
        if (**s == ',') {
          (*s)++;
          skip_whitespace(s);
          if (*s == end || **s == ']') {
            return E_EXPECTED_ARRAY_ELEMENT;
          }
        } else {
          return E_EXPECTED_ARRAY;
        }
      }
      json_array_node *node = new_array_node();
      if (!node)
        return E_STACK_OVERFLOW_ARRAY;
      if (current->u.array.items == NULL) {
        current->u.array.items = node;
      } else {
        current->u.array.last->next = node;
      }
      current->u.array.last = node;
      current = &node->item;
      continue;
    }
    if (current) {
      return E_UNKNOWN_ERROR;
    }
  }
  if (top == -1) {
    if (*s == end)
      return E_NO_ERROR;
    else
      return E_INVALID_DATA;
  }
  return E_INVALID_JSON_DATA;
}

INLINE bool INLINE_ATTRIBUTE json_parse_iterative(const char *s, size_t len, json_value *root) {
  if (*s == '\0')
    return false;
  if (*s != '{' && *s != '[') {
    return false;
  }
  const char *end = s + len;
  json_value *stack[JSON_STACK_SIZE];
  int top = -1;
  json_value *current = root;
  while (true) {
    if (*s == '\0')
      break;
    skip_whitespace(&s);
    if (current) {
      switch (*s) {
      case '{':
        current->type = J_OBJECT;
        current->u.object.items = NULL;
        current->u.object.last = NULL;
        s++;
        if (++top >= JSON_STACK_SIZE)
          return false;
        stack[top] = current;
        current = NULL;
        break;
      case '[':
        current->type = J_ARRAY;
        current->u.array.items = NULL;
        current->u.array.last = NULL;
        s++;
        if (++top >= JSON_STACK_SIZE)
          return false;
        stack[top] = current;
        current = NULL;
        break;
      case '\"':
        current->type = J_STRING;
        if (!parse_string(&s, end, current))
          return false;
        current = NULL;
        break;
      case 't':
        if (*(s + 1) == 'r' && *(s + 2) == 'u' && *(s + 3) == 'e') {
          current->type = J_BOOLEAN;
          current->u.boolean.ptr = s;
          current->u.boolean.len = 4;
          s += 4;
          current = NULL;
        } else {
          return false;
        }
        break;
      case 'f':
        if (*(s + 1) == 'a' && *(s + 2) == 'l' && *(s + 3) == 's' && *(s + 4) == 'e') {
          current->type = J_BOOLEAN;
          current->u.boolean.ptr = s;
          current->u.boolean.len = JSON_FALSE_LEN;
          s += JSON_FALSE_LEN;
          current = NULL;
        } else {
          return false;
        }
        break;
      case 'n':
        if (*(s + 1) == 'u' && *(s + 2) == 'l' && *(s + 3) == 'l') {
          current->type = J_NULL;
          s += 4;
          current = NULL;
        } else {
          return false;
        }
        break;
      default:
        if (parse_number(&s, current)) {
          current->type = J_NUMBER;
          current = NULL;
        } else {
          return false;
        }
      }
      continue;
    }
    if (top == -1) {
      break;
    }
    current = stack[top];
    if (current->type == J_OBJECT) {
      if (*s == '}') {
        s++;
        top--;
        current = NULL;
        continue;
      }
      if (current->u.object.items != NULL) {
        if (*s == ',') {
          s++;
          skip_whitespace(&s);
        } else {
          return false;
        }
      }
      if (*s != '\"')
        return false;
      json_value key;
      if (!parse_string(&s, end, &key))
        return false;
      skip_whitespace(&s);
      if (*s != ':')
        return false;
      s++;
      json_object_node *node = new_object_node();
      if (!node)
        return false;
      node->item.key = key.u.string;
      if (current->u.object.items == NULL) {
        current->u.object.items = node;
      } else {
        current->u.object.last->next = node;
      }
      current->u.object.last = node;
      current = &node->item.value;
    } else if (current->type == J_ARRAY) {
      if (*s == ']') {
        s++;
        top--;
        current = NULL;
        continue;
      }
      if (current->u.array.items != NULL) {
        if (*s == ',') {
          s++;
        } else {
          return false;
        }
      }
      json_array_node *node = new_array_node();
      if (!node)
        return false;
      if (current->u.array.items == NULL) {
        current->u.array.items = node;
      } else {
        current->u.array.last->next = node;
      }
      current->u.array.last = node;
      current = &node->item;
    }
  }
  return *s == '\0' && top == -1;
}

INLINE bool INLINE_ATTRIBUTE json_parse(const char *s, size_t len, json_value *root) {
  if (*s == '\0')
    return false;
  if (len == 0)
    return false;
  if (*s != '{' && *s != '[') {
    return false;
  }
  const char *end = s + len;
  return parse_value_build(&s, end, root) && *s == '\0';
}

bool json_equal(const json_value *a, const json_value *b) {
  if (a == b)
    return true;
  if (!a || !b)
    return false;
  if (a->type != b->type)
    return false;
  switch (a->type) {
  case J_NULL:
    return true;
  case J_BOOLEAN:
    return a->u.boolean.len == b->u.boolean.len && strncmp(a->u.boolean.ptr, b->u.boolean.ptr, a->u.boolean.len) == 0;
  case J_NUMBER: {
    return a->u.number.len == b->u.number.len && strncmp(a->u.number.ptr, b->u.number.ptr, a->u.number.len) == 0;
  }
  case J_STRING:
    if (a->u.string.ptr == NULL && b->u.string.ptr == NULL)
      return true;
    return a->u.string.ptr && b->u.string.ptr && a->u.string.len == b->u.string.len && strncmp(a->u.string.ptr, b->u.string.ptr, a->u.string.len) == 0;
  case J_ARRAY:
    return json_array_equal(a, b);
  case J_OBJECT:
    return json_object_equal(a, b);
  default:
    return false;
  }
}

INLINE void INLINE_ATTRIBUTE json_reset(void) {
  next_array_index = 0;
  next_object_index = 0;
}

INLINE void INLINE_ATTRIBUTE json_cleanup(void) {
  memset(json_array_node_pool, 0, JSON_VALUE_POOL_SIZE * sizeof(json_array_node));
  memset(json_object_node_pool, 0, JSON_VALUE_POOL_SIZE * sizeof(json_object_node));
}

void json_free(json_value *v) {
  json_array_node *array_node = v->u.array.items;
  json_object_node *object_node = v->u.object.items;
  switch (v->type) {
  case J_STRING:
    break;
  case J_ARRAY:
    while (array_node) {
      json_array_node *next = array_node->next;
      json_free(&array_node->item);
      free_array_node(array_node);
      array_node = next;
    }
    v->u.array.items = NULL;
    v->u.array.last = NULL;
    break;
  case J_OBJECT:
    while (object_node) {
      json_object_node *next = object_node->next;
      json_free(&object_node->item.value);
      free_object_node(object_node);
      object_node = next;
    }
    v->u.object.items = NULL;
    v->u.object.last = NULL;
    break;
  default:
    break;
  }
  v->type = J_NULL;
}

INLINE void INLINE_ATTRIBUTE json_print(const json_value *v, FILE *out) {
  if (!v) {
    return;
  }
  print_value(v, 0, out);
}

INLINE const char *INLINE_ATTRIBUTE json_error_string(json_error error) {
  switch (error) {
  case E_NO_ERROR:
    return "No error occurred";
  case E_NO_DATA:
    return "No data provided or empty input";
  case E_INVALID_JSON:
    return "Invalid JSON structure";
  case E_INVALID_JSON_DATA:
    return "Invalid data within JSON structure";
  case E_STACK_OVERFLOW_OBJECT:
    return "Stack overflow while parsing object";
  case E_STACK_OVERFLOW_ARRAY:
    return "Stack overflow while parsing array";
  case E_OBJECT_KEY:
    return "Invalid object key format";
  case E_OBJECT_VALUE:
    return "Invalid object value";
  case E_EXPECTED_OBJECT:
    return "Expected object but found different type";
  case E_EXPECTED_ARRAY:
    return "Expected array but found different type";
  case E_EXPECTED_STRING:
    return "Expected string but found different type";
  case E_EXPECTED_BOOLEAN:
    return "Expected boolean but found different type";
  case E_EXPECTED_NULL:
    return "Expected null but found different type";
  case E_INVALID_DATA:
    return "Invalid data format";
  case E_MAILFORMED_JSON:
    return "Malformed JSON structure";
  case E_UNKNOWN_ERROR:
    return "Unknown or unexpected error";
  case E_NULL:
    return "Null pointer encountered";
  case E_EXPECTED_OBJECT_KEY:
    return "Object key with null pointer";
  case E_EXPECTED_OBJECT_VALUE:
    return "Object value with null pointer";
  case E_EXPECTED_ARRAY_ELEMENT:
    return "Array element with null pointer";
  case E_EXPECTED_OBJECT_ELEMENT:
    return "Object element with null pointer";
  default:
    return "Invalid error code";
  }
}
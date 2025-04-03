/*
 * Copyright (c) 2009-2014 Kazuho Oku, Tokuhiro Matsuno, Daisuke Murase,
 *                         Shigeo Mitsunari
 *
 * The software is licensed under either the MIT License (below) or the Perl
 * license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */// picohttpparser (modified) https://github.com/h2o/picohttpparser
#ifndef picohttpparser_h
#define picohttpparser_h
#include <sys/types.h>
 /* contains name and value of a header (name == NULL if is a continuing line of a multiline header */
#include "hh/str_map.hh"
#include "hpp/hash_map.hpp"
//sv_hash_map vs sv_unordered_map. Just modify it to compare?
namespace fc {
  // using sv_map = sv_unordered_map;
  using sv_map = sv_hash_map<>;
}
#ifdef _WIN32
#define _ssize_t long long
#else
#define _ssize_t long
#endif
 /* returns number of bytes consumed if successful, -2 if request is partial, -1 if failed */
int phr_parse_request(const char* buf, size_t len, const char** method, size_t* method_len, const char** path, size_t* path_len,
  int* minor_version, fc::sv_map* headers, _ssize_t* c_l, size_t last_len);
int phr_parse_response(const char* _buf, size_t len, int* minor_version, int* status, const char** msg, size_t* msg_len,
  fc::sv_map* headers, _ssize_t* c_l, size_t last_len); /* ditto */
int phr_parse_headers(const char* buf, size_t len, fc::sv_map* headers, _ssize_t* c_l, size_t last_len);
#ifdef __cplusplus
extern "C" {
#endif
  /* should be zero-filled before start */
  struct phr_chunked_decoder {
    size_t bytes_left_in_chunk; /* number of bytes left in current chunk */
    char consume_trailer/* if trailing headers should be consumed */, _hex_count, _state;
  };
  /* the function rewrites the buffer given as (buf, bufsz) removing the chunked-
   * encoding headers.  When the function returns without an error, bufsz is
   * updated to the length of the decoded data available.  Applications should
   * repeatedly call the function while it returns -2 (incomplete) every time
   * supplying newly arrived data.  If the end of the chunked-encoded data is
   * found, the function returns a non-negative number indicating the number of
   * octets left undecoded, that starts from the offset returned by `*bufsz`.
   * Returns -1 on error.
   */
  ssize_t phr_decode_chunked(struct phr_chunked_decoder* decoder, char* buf, size_t* bufsz);
  /* returns if the chunked decoder is in middle of chunked data */
  int phr_decode_chunked_is_in_data(struct phr_chunked_decoder* decoder);
#ifdef __cplusplus
}
#endif
#endif

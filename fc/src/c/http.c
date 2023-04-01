#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "h/llhttp.h"
#define CALLBACK_MAYBE(PARSER, NAME)                                          \
  do {                                                                        \
    const llhttp_settings_t* settings;                                        \
    settings = (const llhttp_settings_t*) (PARSER)->settings;                 \
    if (settings == NULL || settings->NAME == NULL) {                         \
      err = 0;break;                                                          \
    }                                                                         \
    err = settings->NAME((PARSER));                                           \
  } while (0)

#define SPAN_CALLBACK_MAYBE(PARSER, NAME, START, LEN)                         \
  do {                                                                        \
    const llhttp_settings_t* settings;                                        \
    settings = (const llhttp_settings_t*) (PARSER)->settings;                 \
    if (settings == NULL || settings->NAME == NULL) {                         \
      err = 0;  break;                                                        \
    }                                                                         \
    err = settings->NAME((PARSER), (START), (LEN));                           \
    if (err == -1) {                                                          \
      err = HPE_USER;                                                         \
      llhttp_set_error_reason((PARSER),"Span callback error in " #NAME);     \
    }                                                                         \
  } while (0)
void llhttp_init(llhttp__internal_t* parser, llhttp_type_t type,
  const llhttp_settings_t* settings) {
  llhttp__internal_init(parser);
  parser->type = type;
  parser->settings = (void*)settings;
}
void llhttp_reset(llhttp__internal_t* parser) {
  int type = parser->type;
  const void* settings = parser->settings;
  void* data = parser->data;
  uint8_t lenient_flags = parser->lenient_flags;
  llhttp__internal_init(parser);
  parser->type = type;
  parser->settings = (void*)settings;
  parser->data = data;
  parser->lenient_flags = lenient_flags;
}
int llhttp_execute(llhttp__internal_t* parser, const char* data, size_t len) { return llhttp__internal_execute(parser, data, data + len); }
void llhttp_settings_init(llhttp_settings_t* settings) { memset(settings, 0, sizeof(*settings)); }
int llhttp_finish(llhttp__internal_t* parser) {
  int err;
  if (parser->error != 0) return HPE_OK;
  switch (parser->finish) {
  case HTTP_FINISH_SAFE_WITH_CB:
    CALLBACK_MAYBE(parser, on_message_complete);
    if (err != HPE_OK) return err;
  case HTTP_FINISH_SAFE:
    return HPE_OK;
  case HTTP_FINISH_UNSAFE:
    parser->reason = "Invalid EOF state";
    return HPE_INVALID_EOF_STATE;
  default:abort();
  }
}
void llhttp_pause(llhttp__internal_t* parser) {
  if (parser->error != HPE_OK) return;
  parser->error = HPE_PAUSED;
  parser->reason = "Paused";
}
void llhttp_resume(llhttp__internal_t* parser) {
  if (parser->error != HPE_PAUSED) return;
  parser->error = 0;
}
void llhttp_resume_after_upgrade(llhttp__internal_t* parser) {
  if (parser->error != HPE_PAUSED_UPGRADE) return;
  parser->error = 0;
}
int llhttp_get_errno(const llhttp__internal_t* parser) { return parser->error; }
const char* llhttp_get_error_reason(const llhttp__internal_t* parser) { return parser->reason; }
void llhttp_set_error_reason(llhttp__internal_t* parser, const char* reason) { parser->reason = reason; }
const char* llhttp_get_error_pos(const llhttp__internal_t* parser) { return parser->error_pos; }
const char* llhttp_errno_name(llhttp_errno_t err) {
#define HTTP_ERRNO_GEN(CODE, NAME, _) case HPE_##NAME: return "HPE_" #NAME;
  switch (err) {
    HTTP_ERRNO_MAP(HTTP_ERRNO_GEN)
  default: abort();
  }
#undef HTTP_ERRNO_GEN
}
const char* llhttp_method_name(llhttp_method_t method) {
#define HTTP_METHOD_GEN(NUM, NAME, STRING) case HTTP_##NAME: return #STRING;
  switch (method) {
    HTTP_ALL_METHOD_MAP(HTTP_METHOD_GEN)
  default: abort();
  }
#undef HTTP_METHOD_GEN
}
void llhttp_set_lenient_headers(llhttp__internal_t* parser, int enabled) { if (enabled) parser->lenient_flags |= LENIENT_HEADERS; else parser->lenient_flags &= ~LENIENT_HEADERS; }
void llhttp_set_lenient_chunked_length(llhttp__internal_t* parser, int enabled) { if (enabled) parser->lenient_flags |= LENIENT_CHUNKED_LENGTH; else parser->lenient_flags &= ~LENIENT_CHUNKED_LENGTH; }
void llhttp_set_lenient_keep_alive(llhttp__internal_t* parser, int enabled) { if (enabled) parser->lenient_flags |= LENIENT_KEEP_ALIVE; else parser->lenient_flags &= ~LENIENT_KEEP_ALIVE; }
int llhttp__on_message_begin(llhttp__internal_t* s, const unsigned char* p, const unsigned char* endp) {
  int err; CALLBACK_MAYBE(s, on_message_begin); return err;
}
int llhttp__on_url(llhttp__internal_t* s, const unsigned char* p, const unsigned char* endp) {
  int err; SPAN_CALLBACK_MAYBE(s, on_url, (const char*)p, endp - p); return err;
}
int llhttp__on_url_complete(llhttp__internal_t* s, const unsigned char* p, const unsigned char* endp) {
  int err; CALLBACK_MAYBE(s, on_url_complete); return err;
}
int llhttp__on_status(llhttp__internal_t* s, const unsigned char* p, const unsigned char* endp) {
  int err; SPAN_CALLBACK_MAYBE(s, on_status, (const char*)p, endp - p); return err;
}
int llhttp__on_status_complete(llhttp__internal_t* s, const unsigned char* p, const unsigned char* endp) {
  int err; CALLBACK_MAYBE(s, on_status_complete); return err;
}
int llhttp__on_header_field(llhttp__internal_t* s, const unsigned char* p, const unsigned char* endp) {
  int err; SPAN_CALLBACK_MAYBE(s, on_header_field, (const char*)p, endp - p); return err;
}
int llhttp__on_header_field_complete(llhttp__internal_t* s, const unsigned char* p, const unsigned char* endp) {
  int err; CALLBACK_MAYBE(s, on_header_field_complete); return err;
}
int llhttp__on_header_value(llhttp__internal_t* s, const unsigned char* p, const unsigned char* endp) {
  int err; SPAN_CALLBACK_MAYBE(s, on_header_value, (const char*)p, endp - p); return err;
}
int llhttp__on_header_value_complete(llhttp__internal_t* s, const unsigned char* p, const unsigned char* endp) {
  int err; CALLBACK_MAYBE(s, on_header_value_complete); return err;
}
int llhttp__on_headers_complete(llhttp__internal_t* s, const unsigned char* p, const unsigned char* endp) {
  int err; CALLBACK_MAYBE(s, on_headers_complete); return err;
}
int llhttp__on_message_complete(llhttp__internal_t* s, const unsigned char* p, const unsigned char* endp) {
  int err; CALLBACK_MAYBE(s, on_message_complete); return err;
}
int llhttp__on_body(llhttp__internal_t* s, const unsigned char* p, const unsigned char* endp) {
  int err; SPAN_CALLBACK_MAYBE(s, on_body, (const char*)p, endp - p); return err;
}
int llhttp__on_chunk_header(llhttp__internal_t* s, const unsigned char* p, const unsigned char* endp) {
  int err; CALLBACK_MAYBE(s, on_chunk_header); return err;
}
int llhttp__on_chunk_complete(llhttp__internal_t* s, const unsigned char* p, const unsigned char* endp) {
  int err; CALLBACK_MAYBE(s, on_chunk_complete); return err;
}
int llhttp__before_headers_complete(llhttp__internal_t* parser, const unsigned char* p,
  const unsigned char* endp) {
  if ((parser->flags & F_UPGRADE) &&
    (parser->flags & F_CONNECTION_UPGRADE)) {
    parser->upgrade =
      (parser->type == HTTP_REQUEST || parser->status_code == 101);
  } else { parser->upgrade = (parser->method == HTTP_CONNECT); }
  return 0;
}
int llhttp__after_headers_complete(llhttp__internal_t* parser, const unsigned char* p,
  const unsigned char* endp) {
  int hasBody;
  hasBody = parser->flags & F_CHUNKED || parser->content_length > 0;
  if (parser->upgrade && (parser->method == HTTP_CONNECT ||
    (parser->flags & F_SKIPBODY) || !hasBody)) {
    return 1;
  }
  if (parser->flags & F_SKIPBODY) return 0; else if (parser->flags & F_CHUNKED) return 2; else if (parser->flags & F_TRANSFER_ENCODING) {
    if (parser->type == HTTP_REQUEST &&
      (parser->lenient_flags & LENIENT_CHUNKED_LENGTH) == 0) {
      return 5;
    } else { return 4; }
  } else {
    if (!(parser->flags & F_CONTENT_LENGTH)) { if (!llhttp_message_needs_eof(parser)) return 0; else return 4; } else if (parser->content_length == 0) return 0; else return 3;
  }
}
int llhttp__after_message_complete(llhttp__internal_t* parser, const unsigned char* p,
  const unsigned char* endp) {
  int should_keep_alive;
  should_keep_alive = llhttp_should_keep_alive(parser);
  parser->finish = HTTP_FINISH_SAFE;
  parser->flags = 0;
  return should_keep_alive;
}
int llhttp_message_needs_eof(const llhttp__internal_t* parser) {
  if (parser->type == HTTP_REQUEST) return 0;
  if (parser->status_code / 100 == 1 ||
    parser->status_code == 204 ||
    parser->status_code == 304 ||
    (parser->flags & F_SKIPBODY)) {
    return 0;
  }
  if ((parser->flags & F_TRANSFER_ENCODING) &&
    (parser->flags & F_CHUNKED) == 0) {
    return 1;
  }
  if (parser->flags & (F_CHUNKED | F_CONTENT_LENGTH)) return 0;
  return 1;
}
int llhttp_should_keep_alive(const llhttp__internal_t* parser) {
  if (parser->http_major > 0 && parser->http_minor > 0) { if (parser->flags & F_CONNECTION_CLOSE) return 0; } else { if (!(parser->flags & F_CONNECTION_KEEP_ALIVE)) return 0; }
  return !llhttp_message_needs_eof(parser);
}
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "h/llhttp.h"
#define CALLBACK_MAYBE(PARSER, NAME)                                          \
  do {                                                                        \
    const llhttp_settings_t* settings;                                        \
    settings = (const llhttp_settings_t*) (PARSER)->settings;                 \
    if (settings == NULL || settings->NAME == NULL) {                         \
      err = 0;                                                                \
      break;                                                                  \
    }                                                                         \
    err = settings->NAME((PARSER));                                           \
  } while (0)
#define SPAN_CALLBACK_MAYBE(PARSER, NAME, START, LEN)                         \
  do {                                                                        \
    const llhttp_settings_t* settings;                                        \
    settings = (const llhttp_settings_t*) (PARSER)->settings;                 \
    if (settings == NULL || settings->NAME == NULL) {                         \
      err = 0;                                                                \
      break;                                                                  \
    }                                                                         \
    err = settings->NAME((PARSER), (START), (LEN));                           \
    if (err == -1) {                                                          \
      err = HPE_USER;                                                         \
      llhttp_set_error_reason((PARSER), "Span callback error in " #NAME);     \
    }                                                                         \
  } while (0)
void llhttp_init(llhttp_t* parser, llhttp_type_t type,
				 const llhttp_settings_t* settings) {
  llhttp__internal_init(parser);
  parser->type = type;
  parser->settings = (void*)settings;
}
void llhttp_reset(llhttp_t* parser) {
  llhttp_type_t type = parser->type;
  const llhttp_settings_t* settings = parser->settings;
  void* data = parser->data;
  uint8_t lenient_flags = parser->lenient_flags;
  llhttp__internal_init(parser);
  parser->type = type;
  parser->settings = (void*)settings;
  parser->data = data;
  parser->lenient_flags = lenient_flags;
}
llhttp_errno_t llhttp_execute(llhttp_t* parser, const char* data, size_t len) {
  return llhttp__internal_execute(parser, data, data + len);
}
void llhttp_settings_init(llhttp_settings_t* settings) {
  memset(settings, 0, sizeof(*settings));
}
llhttp_errno_t llhttp_finish(llhttp_t* parser) {
  int err;
  /* We're in an error state. Don't bother doing anything. */
  if (parser->error != 0) {
	return 0;
  }
  switch (parser->finish) {
  case HTTP_FINISH_SAFE_WITH_CB:
	CALLBACK_MAYBE(parser, on_message_complete);
	if (err != HPE_OK) return err;
	/* FALLTHROUGH */
  case HTTP_FINISH_SAFE:
	return HPE_OK;
  case HTTP_FINISH_UNSAFE:
	parser->reason = "Invalid EOF state";
	return HPE_INVALID_EOF_STATE;
  default:
	abort();
  }
}
void llhttp_pause(llhttp_t* parser) {
  if (parser->error != HPE_OK) {
	return;
  }
  parser->error = HPE_PAUSED;
  parser->reason = "Paused";
}
void llhttp_resume(llhttp_t* parser) {
  if (parser->error != HPE_PAUSED) {
	return;
  }
  parser->error = 0;
}
void llhttp_resume_after_upgrade(llhttp_t* parser) {
  if (parser->error != HPE_PAUSED_UPGRADE) {
	return;
  }
  parser->error = 0;
}
llhttp_errno_t llhttp_get_errno(const llhttp_t* parser) {
  return parser->error;
}
const char* llhttp_get_error_reason(const llhttp_t* parser) {
  return parser->reason;
}
void llhttp_set_error_reason(llhttp_t* parser, const char* reason) {
  parser->reason = reason;
}
const char* llhttp_get_error_pos(const llhttp_t* parser) {
  return parser->error_pos;
}
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
void llhttp_set_lenient_headers(llhttp_t* parser, int enabled) {
  if (enabled) {
	parser->lenient_flags |= LENIENT_HEADERS;
  } else {
	parser->lenient_flags &= ~LENIENT_HEADERS;
  }
}
void llhttp_set_lenient_chunked_length(llhttp_t* parser, int enabled) {
  if (enabled) {
	parser->lenient_flags |= LENIENT_CHUNKED_LENGTH;
  } else {
	parser->lenient_flags &= ~LENIENT_CHUNKED_LENGTH;
  }
}
void llhttp_set_lenient_keep_alive(llhttp_t* parser, int enabled) {
  if (enabled) {
	parser->lenient_flags |= LENIENT_KEEP_ALIVE;
  } else {
	parser->lenient_flags &= ~LENIENT_KEEP_ALIVE;
  }
}
/* Callbacks */
int llhttp__on_message_begin(llhttp_t* s, const char* p, const char* endp) {
  int err;
  CALLBACK_MAYBE(s, on_message_begin);
  return err;
}
int llhttp__on_url(llhttp_t* s, const char* p, const char* endp) {
  int err;
  SPAN_CALLBACK_MAYBE(s, on_url, p, endp - p);
  return err;
}
int llhttp__on_url_complete(llhttp_t* s, const char* p, const char* endp) {
  int err;
  CALLBACK_MAYBE(s, on_url_complete);
  return err;
}
int llhttp__on_status(llhttp_t* s, const char* p, const char* endp) {
  int err;
  SPAN_CALLBACK_MAYBE(s, on_status, p, endp - p);
  return err;
}
int llhttp__on_status_complete(llhttp_t* s, const char* p, const char* endp) {
  int err;
  CALLBACK_MAYBE(s, on_status_complete);
  return err;
}
int llhttp__on_header_field(llhttp_t* s, const char* p, const char* endp) {
  int err;
  SPAN_CALLBACK_MAYBE(s, on_header_field, p, endp - p);
  return err;
}
int llhttp__on_header_field_complete(llhttp_t* s, const char* p, const char* endp) {
  int err;
  CALLBACK_MAYBE(s, on_header_field_complete);
  return err;
}
int llhttp__on_header_value(llhttp_t* s, const char* p, const char* endp) {
  int err;
  SPAN_CALLBACK_MAYBE(s, on_header_value, p, endp - p);
  return err;
}
int llhttp__on_header_value_complete(llhttp_t* s, const char* p, const char* endp) {
  int err;
  CALLBACK_MAYBE(s, on_header_value_complete);
  return err;
}
int llhttp__on_headers_complete(llhttp_t* s, const char* p, const char* endp) {
  int err;
  CALLBACK_MAYBE(s, on_headers_complete);
  return err;
}
int llhttp__on_message_complete(llhttp_t* s, const char* p, const char* endp) {
  int err;
  CALLBACK_MAYBE(s, on_message_complete);
  return err;
}
int llhttp__on_body(llhttp_t* s, const char* p, const char* endp) {
  int err;
  SPAN_CALLBACK_MAYBE(s, on_body, p, endp - p);
  return err;
}
int llhttp__on_chunk_header(llhttp_t* s, const char* p, const char* endp) {
  int err;
  CALLBACK_MAYBE(s, on_chunk_header);
  return err;
}
int llhttp__on_chunk_complete(llhttp_t* s, const char* p, const char* endp) {
  int err;
  CALLBACK_MAYBE(s, on_chunk_complete);
  return err;
}
/* Private */
void llhttp__debug(llhttp_t* s, const char* p, const char* endp,
				   const char* msg) {
  if (p == endp) {
	fprintf(stderr, "p=%p type=%d flags=%02x next=null debug=%s\n", s, s->type,
			s->flags, msg);
  } else {
	fprintf(stderr, "p=%p type=%d flags=%02x next=%02x   debug=%s\n", s,
			s->type, s->flags, *p, msg);
  }
}
int llhttp_message_needs_eof(const llhttp_t* parser);
int llhttp_should_keep_alive(const llhttp_t* parser);
int llhttp__before_headers_complete(llhttp_t* parser, const char* p,
									const char* endp) {
  /* Set this here so that on_headers_complete() callbacks can see it */
  if ((parser->flags & F_UPGRADE) &&
	  (parser->flags & F_CONNECTION_UPGRADE)) {
	/* For responses, "Upgrade: foo" and "Connection: upgrade" are
	 * mandatory only when it is a 101 Switching Protocols response,
	 * otherwise it is purely informational, to announce support.
	 */
	parser->upgrade =
	  (parser->type == HTTP_REQUEST || parser->status_code == 101);
  } else {
	parser->upgrade = (parser->method == HTTP_CONNECT);
  }
  return 0;
}
/* Return values:
 * 0 - No body, `restart`, message_complete
 * 1 - CONNECT request, `restart`, message_complete, and pause
 * 2 - chunk_size_start
 * 3 - body_identity
 * 4 - body_identity_eof
 * 5 - invalid transfer-encoding for request
 */
int llhttp__after_headers_complete(llhttp_t* parser, const char* p,
								   const char* endp) {
  int hasBody;
  hasBody = parser->flags & F_CHUNKED || parser->content_length > 0;
  if (parser->upgrade && (parser->method == HTTP_CONNECT ||
	(parser->flags & F_SKIPBODY) || !hasBody)) {
	/* Exit, the rest of the message is in a different protocol. */
	return 1;
  }
  if (parser->flags & F_SKIPBODY) {
	return 0;
  } else if (parser->flags & F_CHUNKED) {
	/* chunked encoding - ignore Content-Length header, prepare for a chunk */
	return 2;
  } else if (parser->flags & F_TRANSFER_ENCODING) {
	if (parser->type == HTTP_REQUEST &&
		(parser->lenient_flags & LENIENT_CHUNKED_LENGTH) == 0) {
	  /* RFC 7230 3.3.3 */
	  /* If a Transfer-Encoding header field
	   * is present in a request and the chunked transfer coding is not
	   * the final encoding, the message body length cannot be determined
	   * reliably; the server MUST respond with the 400 (Bad Request)
	   * status code and then close the connection.
	   */
	  return 5;
	} else {
	  /* RFC 7230 3.3.3 */
	  /* If a Transfer-Encoding header field is present in a response and
	   * the chunked transfer coding is not the final encoding, the
	   * message body length is determined by reading the connection until
	   * it is closed by the server.
	   */
	  return 4;
	}
  } else {
	if (!(parser->flags & F_CONTENT_LENGTH)) {
	  if (!llhttp_message_needs_eof(parser)) {
		/* Assume content-length 0 - read the next */
		return 0;
	  } else {
		/* Read body until EOF */
		return 4;
	  }
	} else if (parser->content_length == 0) {
	  /* Content-Length header given but zero: Content-Length: 0\r\n */
	  return 0;
	} else {
	  /* Content-Length header given and non-zero */
	  return 3;
	}
  }
}
int llhttp__after_message_complete(llhttp_t* parser, const char* p,
								   const char* endp) {
  int should_keep_alive;
  should_keep_alive = llhttp_should_keep_alive(parser);
  parser->finish = HTTP_FINISH_SAFE;
  parser->flags = 0;
  /* NOTE: this is ignored in loose parsing mode */
  return should_keep_alive;
}
int llhttp_message_needs_eof(const llhttp_t* parser) {
  if (parser->type == HTTP_REQUEST) {
	return 0;
  }
  /* See RFC 2616 section 4.4 */
  if (parser->status_code / 100 == 1 || /* 1xx e.g. Continue */
	  parser->status_code == 204 ||     /* No Content */
	  parser->status_code == 304 ||     /* Not Modified */
	  (parser->flags & F_SKIPBODY)) {     /* response to a HEAD request */
	return 0;
  }
  /* RFC 7230 3.3.3, see `llhttp__after_headers_complete` */
  if ((parser->flags & F_TRANSFER_ENCODING) &&
	  (parser->flags & F_CHUNKED) == 0) {
	return 1;
  }
  if (parser->flags & (F_CHUNKED | F_CONTENT_LENGTH)) {
	return 0;
  }
  return 1;
}
int llhttp_should_keep_alive(const llhttp_t* parser) {
  if (parser->http_major > 0 && parser->http_minor > 0) {
	/* HTTP/1.1 */
	if (parser->flags & F_CONNECTION_CLOSE) {
	  return 0;
	}
  } else {
	/* HTTP/1.0 or earlier */
	if (!(parser->flags & F_CONNECTION_KEEP_ALIVE)) {
	  return 0;
	}
  }
  return !llhttp_message_needs_eof(parser);
}

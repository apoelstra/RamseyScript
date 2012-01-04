#ifndef STREAM_H
#define STREAM_H

#include <stddef.h>

enum e_stream_type {
  STREAM_READ = 1,
  STREAM_WRITE = 2,
  STREAM_APPEND = 4
};

typedef struct _stream {
  void *_data;
  enum e_stream_type type;
  void *(*open)       (struct _stream *, void *);
  void  (*close)      (struct _stream *);
  char *(*read_line)  (struct _stream *);
  int   (*write_line) (struct _stream *, const char *);
  int   (*eof)        (struct _stream *);
  void  (*destroy)    (struct _stream *);
} Stream;

Stream *file_stream_new (const char *mode);
void stream_line_copy (Stream *output, Stream *input);
void stream_printf (Stream *output, const char *fmt, ...);

#endif

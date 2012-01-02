#ifndef STREAM_H
#define STREAM_H

#include <stddef.h>

enum e_stream_type {
  STREAM_READ,
  STREAM_WRITE,
  STREAM_APPEND
};

typedef struct _stream {
  void *_data;
  enum e_stream_type type;
  void *(*open)      (struct _stream *, void *);
  void  (*close)     (struct _stream *);
  char *(*read_line) (struct _stream *);
  int   (*eof)       (struct _stream *);
} Stream;

Stream *file_stream_new (const char *mode);
void file_stream_delete (Stream *stream);

#endif

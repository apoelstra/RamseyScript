#ifndef FILE_STREAM_H
#define FILE_STREAM_H

#include "stream.h"

stream_t *file_stream_new (const char *filename);
stream_t *stdout_stream_new ();
stream_t *stderr_stream_new ();
stream_t *stdin_stream_new  ();

#endif

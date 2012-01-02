#ifndef GTK_TEXT_BUFFER_STREAM_H
#define GTK_TEXT_BUFFER_STREAM_H

Stream *text_buffer_stream_new (GtkTextBuffer *buff);
void text_buffer_stream_delete (Stream *stream);

#endif

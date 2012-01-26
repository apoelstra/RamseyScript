#ifndef PROCESS_H
#define PROCESS_H

struct _global_data *set_defaults (stream_t *in, stream_t *out, stream_t *err);
void process (struct _global_data *state);

#endif

#ifndef SEQUENCE_H
#define SEQUENCE_H

#define DEFAULT_MAX_LENGTH	400

typedef struct {
  int *values;
  int length;
  int _max_length;
} Sequence;

Sequence *sequence_new ();
int sequence_append (Sequence *s, int value);
void sequence_deappend (Sequence *s);
void sequence_print (Sequence *s);
void sequence_delete (Sequence *s);

#define sequence_max(s) ((s)->values[(s)->length - 1])

#endif

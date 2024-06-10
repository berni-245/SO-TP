#ifndef PIPES_H
#define PIPES_H

typedef struct {
  int write;
  int read;
  int err;
} ProcessPipes;

#endif

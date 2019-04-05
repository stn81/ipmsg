#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>
#include <stdio.h>

extern void *s_malloc(size_t size);
extern char *fget_str(char *s, int n, FILE *stream);
#endif

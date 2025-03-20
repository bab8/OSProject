#ifndef _LIB_H_
#define _LIB_H_

#include "stdint.h"

int printf(const char *format, ...);
void waitu(void);
void exitu(void);
void sleepu(uint64_t ticks);

#endif
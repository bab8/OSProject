#ifndef _LIB_H_
#define _LIB_H_

void memset(void* buffer, char value, int size);
void memmove(void* dest, void* src, int size);
void memcpy(void* dest, void* src, int size);
int memcmp(void* src1, void* src2, int size);

#endif
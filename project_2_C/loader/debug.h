#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "stdint.h"

//if expresseion evals to false, pass file path and line number to erro check function
#define ASSERT(e) do {              \
    if(!(e))                        \
    error_check(__FILE__,__LINE__); \
}while(0)

void error_check(char* file, uint64_t line);

#endif
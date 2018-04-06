#ifndef __philosopher_program_H
#define __philosopher_program_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <string.h>

#include "PL011.h"

#include "libc.h"
#include "console.h"

typedef struct {
    int             read;
    int             write;
} pfd_t;

#endif

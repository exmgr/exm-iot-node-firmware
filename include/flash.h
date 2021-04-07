#ifndef FLASH_H
#define FLASH_H
#include <inttypes.h>
#include "struct.h"

namespace Flash
{
    RetResult mount();

    RetResult read_file(const char *path, uint8_t *dest, int bytes);

    RetResult format();

    void ls();
}

#endif
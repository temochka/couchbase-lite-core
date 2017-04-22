#pragma once
#include "Export.h"

#ifdef __cplusplus
extern "C" {
#endif

    PLATFORM_DLL void arc4random_buf(void *buffer, int size);

    PLATFORM_DLL uint32_t arc4random();

#ifdef __cplusplus
}
#endif
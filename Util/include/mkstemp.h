#pragma once
#include "Export.h"

#ifdef __cplusplus
extern "C" {
#endif
    PLATFORM_DLL int mkstemp(char *tmpl);
#ifdef __cplusplus
}
#endif
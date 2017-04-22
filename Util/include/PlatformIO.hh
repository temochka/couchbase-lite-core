//
//  PlatformIO.hh
//  LiteCore
//
//  Created by Jim Borden on 11/28/16.
//  Copyright © 2016 Couchbase. All rights reserved.
//

#pragma once
#include "PlatformCompat.hh"
#include "Export.h"


#ifdef _MSC_VER
    #include <cstdio>

    #define fdopen      ::_fdopen
    #define fseeko      fseek
    #define ftello      ftell
    #define strncasecmp _strnicmp
    #define strcasecmp  _stricmp
    #define fdclose ::_close

    namespace litecore {
        PLATFORM_DLL int mkdir_u8(const char* const path, int mode);
        PLATFORM_DLL int stat_u8(const char* const filename, struct stat* const s);
        PLATFORM_DLL int rmdir_u8(const char* const path);
        PLATFORM_DLL int rename_u8(const char* const oldPath, const char* const newPath);
        PLATFORM_DLL int unlink_u8(const char* const filename);
        PLATFORM_DLL int chmod_u8(const char* const filename, int mode);
        PLATFORM_DLL FILE* fopen_u8(const char* const path, const char* const mode);
    }

#else

    #include <stdio.h>
    #include <sys/stat.h>
    #include <unistd.h>

    #define fdclose ::close

    namespace litecore {
        inline int mkdir_u8(const char* const path, int mode) {
            return ::mkdir(path, (mode_t)mode);
        }

        inline int stat_u8(const char* const filename, struct ::stat* const s) {
            return ::stat(filename, s);
        }

        inline int rmdir_u8(const char* const path) {
            return ::rmdir(path);
        }

        inline int rename_u8(const char* const oldPath, const char* const newPath) {
            return ::rename(oldPath, newPath);
        }

        inline int unlink_u8(const char* const filename) {
            return ::unlink(filename);
        }

        inline int chmod_u8(const char* const filename, int mode) {
            return ::chmod(filename, (mode_t)mode);
        }

        inline FILE* fopen_u8(const char* const path, const char* const mode) {
            return ::fopen(path, mode);
        }
    }

#endif

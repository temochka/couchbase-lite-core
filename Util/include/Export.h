//
//  PlatformIO.hh
//  LiteCore
//
//  Created by Jim Borden on 11/28/16.
//  Copyright © 2016 Couchbase. All rights reserved.
//

#pragma once

#ifdef _MSC_VER
    #ifdef PLATFORM_IO_EXPORT
    #define PLATFORM_DLL __declspec(dllexport)
    #else
    #define PLATFORM_DLL __declspec(dllimport)
    #endif
#else
    #define PLATFORM_DLL
#endif

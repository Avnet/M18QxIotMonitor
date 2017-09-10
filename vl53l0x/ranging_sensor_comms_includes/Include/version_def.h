// ======================================================================================
//                               COMMS & CAPTURE SDK
// ======================================================================================
// Copyright 2008-2014 STMicroelectronics. All Rights Reserved.
//
// NOTICE:  STMicroelectronics permits you to use this file in accordance with the terms
// of the STMicroelectronics license agreement accompanying it.
// ======================================================================================
#ifndef VERSION_DEF_H
#define VERSION_DEF_H

#include <stdint.h>

typedef struct tag_version_info{
    uint8_t major;
    uint8_t minor;
    uint8_t build;
    uint32_t revision;
} version_info, *pversion_info;

#endif // VERSION_DEF_H

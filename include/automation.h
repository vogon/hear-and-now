/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#pragma once

#ifndef _HN_AUTOMATION_H
#define _HN_AUTOMATION_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum HnCmdCode
{
    CmdNoteOn,
    CmdNoteOff,
} HnCmdCode;

#define HN_COMMAND_COMMON \
    uint32_t sample; \
    HnCmdCode code;

#define COMMAND_START(typename) \
    typedef struct typename \
    { \
        HN_COMMAND_COMMON

#define COMMAND_END(typename) \
    } typename;

COMMAND_START(HnCmd)
COMMAND_END(HnCmd)

COMMAND_START(HnNoteOnCmd)
    float pitch;
COMMAND_END(HnNoteOnCmd)

COMMAND_END(HnNoteOffCmd)
    float pitch;
COMMAND_END(HnNoteOffCmd)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _HN_AUTOMATION_H */

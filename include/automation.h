/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#pragma once

#ifndef _HN_AUTOMATION_H
#define _HN_AUTOMATION_H

#include "hn.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum HnCmdCode
{
    CmdNoteOn = 1,
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

COMMAND_START(HnNoteOffCmd)
    float pitch;
COMMAND_END(HnNoteOffCmd)

struct HnCmdQueue;
typedef struct HnCmdQueue HnCmdQueue;

HnCmdQueue *hn_cmd_queue_create();

void hn_cmd_queue_send(HnCmdQueue *pQueue, HnCmd *pCmd);

HnCmd *hn_cmd_queue_pop(HnCmdQueue *pQueue);

void hn_sequencer_trigger(HnSequencer *pSeq, HnCmdQueue *pQueue, HnCmd *pCmd, 
    jiffies_t jiffy);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _HN_AUTOMATION_H */

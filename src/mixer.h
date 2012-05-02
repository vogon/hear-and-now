/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#pragma once

#ifndef _HN_MIXER_H
#define _HN_MIXER_H

#include "hn.h"
#include "locks.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Stream
{
    struct Stream *pNext, *pPrev;

    void *pContext;
    float volume;
    HnGeneratorFn callback;
} Stream;

typedef struct 
{
    uint64_t samplesMixed;

    HnAudio *pAudio;
    uint32_t audioLowWater;
    HnMutex *pAudioLock;
    HnConditionVariable *pAudioLowWater;

    Stream *pFirstStream, *pLastStream;
    HnMutex *pStreamLock;
} HnMixer_impl;

#ifdef __cplusplus
}
#endif

#endif /* _HN_MIXER_H */
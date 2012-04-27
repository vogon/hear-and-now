/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#pragma once

#ifndef _HN_MIXER_H
#define _HN_MIXER_H

#include "hn.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Stream
{
    struct Stream *pNext, *pPrev;

    void *pContext;
    float volume;
    float *(*callback)(void *, uint32_t);
} Stream;

typedef struct 
{
    HnAudio *pAudio;
    Stream *pFirstStream, *pLastStream;
} HnMixer_impl;

#ifdef __cplusplus
}
#endif

#endif /* _HN_MIXER_H */
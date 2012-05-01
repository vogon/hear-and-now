/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#pragma once

#ifndef _HN_AUDIO_H
#define _HN_AUDIO_H

#include <stdint.h>

#include "hn.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HnAudio_vtbl
{
    void (*watch)(struct HnAudio *, void (*)(void *, uint32_t), void *);
    void (*write)(struct HnAudio *, uint8_t *, uint32_t);
    uint32_t (*samples_pending)(struct HnAudio *);

    void (*close)(struct HnAudio *);
} HnAudio_vtbl;

struct HnAudio
{
    HnAudio_vtbl *pVtbl;
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _HN_AUDIO_H */
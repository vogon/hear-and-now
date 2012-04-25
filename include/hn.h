/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#pragma once

#ifndef _HN_HN_H
#define _HN_HN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HnAudio
{
	void *pImpl;

    void (*close)(struct HnAudio *);
} HnAudio;

HnAudio *hn_audio_open();

typedef struct
{
} HnMixer;

HnMixer *hn_mixer_create(HnAudio *pAudio);

void hn_mixer_release(HnMixer *pMixer);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _HN_HN_H */

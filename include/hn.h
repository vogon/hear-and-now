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

struct HnAudio;
typedef struct HnAudio HnAudio;

HnAudio *hn_audio_open();

void hn_audio_watch(HnAudio *pAudio, void (*callback)(void *, uint32_t), void *context);

void hn_audio_write(HnAudio *pAudio, uint8_t *pData, uint32_t len);

uint32_t hn_audio_samples_pending(HnAudio *pAudio);

void hn_audio_close(HnAudio *pAudio);

typedef struct HnMixer
{
    void *pImpl;
} HnMixer;

HnMixer *hn_mixer_create(HnAudio *pAudio);

void hn_mixer_release(HnMixer *pMixer);

void hn_mixer_add_stream(HnMixer *pMixer, void *pContext,
    float *(*callback)(void *, uint32_t));

void hn_mixer_start(HnMixer *pMixer);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _HN_HN_H */

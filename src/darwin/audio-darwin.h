/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#pragma once

#ifndef _HN_AUDIO_DARWIN_H
#define _HN_AUDIO_DARWIN_H

#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CoreFoundation.h>

#include "hn.h"
#include "audio.h"
#include "locks.h"

#ifdef __cplusplus
extern "C" {
#endif

HnAudio *hn_darwin_audio_open(HnAudioFormat *pFormat);

HnAudioFormat *hn_darwin_audio_format(HnAudio *pAudio);

void hn_darwin_audio_watch(HnAudio *pAudio, void (*callback)(void *, uint32_t), void *context);

void hn_darwin_audio_write(HnAudio *pAudio, uint8_t *pData, uint32_t len);

uint32_t hn_darwin_audio_samples_pending(HnAudio *pAudio);

void hn_darwin_audio_close(HnAudio *pAudio);

#ifdef __cplusplus
}
#endif

#endif /* _HN_AUDIO_DARWIN_H */

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

typedef struct HnAudioFormat
{
    uint32_t samplesPerSecond;
    uint8_t sampleResolution;
    uint8_t numberOfChannels;
} HnAudioFormat;

struct HnAudio;
typedef struct HnAudio HnAudio;

HnAudio *hn_audio_open(HnAudioFormat *pFormat);

void hn_audio_watch(HnAudio *pAudio, void (*callback)(void *, uint32_t), void *context);

void hn_audio_write(HnAudio *pAudio, uint8_t *pData, uint32_t len);

uint32_t hn_audio_samples_pending(HnAudio *pAudio);

void hn_audio_close(HnAudio *pAudio);

typedef struct HnMixer
{
    void *pImpl;
} HnMixer;

typedef float *(*HnGeneratorFn) 
    (void *pContext,
     uint64_t start,
     uint32_t length);

HnMixer *hn_mixer_create(HnAudio *pAudio);

void hn_mixer_release(HnMixer *pMixer);

void hn_mixer_add_stream(HnMixer *pMixer, void *pContext,
    HnGeneratorFn callback, uint8_t priority);

void hn_mixer_start(HnMixer *pMixer);

struct HnSequencer;
typedef struct HnSequencer HnSequencer;

/*
 * a "jiffy" is the length of one 256th-note.
 *
 * at 400bpm 4/4, a 32-bit jiffies count should be enough for 
 * 116 days or so of performance.
 */
typedef uint32_t jiffies_t;

HnSequencer *hn_sequencer_create();

void hn_sequencer_release(HnSequencer *pSeq);

void hn_sequencer_attach(HnSequencer *pSeq, HnMixer *pMixer);

void hn_sequencer_play(HnSequencer *pSeq);

void hn_sequencer_set_bpm(HnSequencer *pSeq, float bpm);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _HN_HN_H */

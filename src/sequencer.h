/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#pragma once

#ifndef _HN_SEQUENCER_H
#define _HN_SEQUENCER_H

#include <stdint.h>

#include "hn.h"

#ifdef __cplusplus
extern "C" {
#endif

#define JIFFIES_PER_WHOLE 256

typedef struct
{
    /* 
     * jiffy tempo = number of jiffies per second. 
     * in 4/4, jiffy tempo = bpm tempo * 256 / (4 * 60).
     */
    float jiffyTempo;
} HnTimeSignature;

typedef struct 
{
    jiffies_t nextJiffy;
    float nextJiffyStart;
} HnSequencerBlockState;

/*
 * attach:
 * - register sync "generator" with mixer
 *
 * trigger:
 * - add scheduling item to priority queue
 *
 * sync generate callback:
 * - update last sample
 * - for each scheduled item:
 *   - convert performance time to sample count
 *   - if sample count < last sample:
 *     - send automation command
 *     - dequeue
 *   - else break
 */
struct HnSequencer
{
    HnMixer *pMixer;

    uint64_t samplesPerformed;
    HnTimeSignature *pSignature;
    HnTimeSignature *pCommandedSignature;
    HnSequencerBlockState *pNextBlock;

    /* test code: block offsets of pending clicks */
    int32_t clicks[10];
    uint32_t nextClick; 
};

float *seq_internal_sync(void *pContext, uint64_t start, uint32_t length);

float *seq_internal_gen_click(void *pContext, uint64_t start, uint32_t length);

void seq_internal_set_tempo(HnSequencer *pSeq, float jiffyTempo);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _HN_SEQUENCER_H */
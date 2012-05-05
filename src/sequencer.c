/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#include "hn.h"
#include "sequencer.h"

// #include <windows.h>

#include <stdlib.h>
#include <memory.h>

#define SAMPLE_RATE 44100

float *seq_internal_gen_click(void *pContext, uint64_t start, uint32_t length)
{
    // printf("ahhhhh\n");

    HnSequencer *pSeq = (HnSequencer *)pContext;
    float *data = (float *)calloc(length, sizeof(float));

    for (int i = 0; i < 10; i++)
    {
        if (pSeq->clicks[i] >= 0)
        {
            data[pSeq->clicks[i]] = 1.0f;
        }
    }

    return data;
}

float *seq_internal_sync(void *pContext, uint64_t start, uint32_t length)
{
    HnSequencer *pSeq = (HnSequencer *)pContext;

    /* switch to any new time signature we got */
    if (pSeq->pSignature != pSeq->pCommandedSignature)
    {
        // printf("meep\n");
        pSeq->pSignature = pSeq->pCommandedSignature;
    }

    /* if signature is null, we're not playing */
    if (NULL != pSeq->pSignature)
    {
        // printf("blah\n");

        /* test code: reset click state */
        memset(pSeq->clicks, 0xff, sizeof(int32_t) * 10);
        pSeq->nextClick = 0;

        float samplesPerJiffy = (float)SAMPLE_RATE / pSeq->pSignature->jiffyTempo;
        jiffies_t jiffy;
        float sample;

        /* wake up everything every jiffy */
        for (jiffy = pSeq->pNextBlock->nextJiffy,
             sample = pSeq->pNextBlock->nextJiffyStart;
             sample < length;
             jiffy++, sample += samplesPerJiffy)
        {
            /* test code: play click if jiffy % 64 == 0 */
            if (jiffy % 64 == 0)
            {
                pSeq->clicks[pSeq->nextClick] = sample;
                pSeq->nextClick++;
                // printf("[%u] click at %u, jiffy %u\n", GetTickCount(), (uint16_t)sample, jiffy);
            }
        }

        /* leave a record for the next block */
        pSeq->pNextBlock->nextJiffy = jiffy;
        pSeq->pNextBlock->nextJiffyStart = sample - length;
    }

    return NULL; /* don't actually generate any audio data */
}

HnSequencer *hn_sequencer_create()
{
    HnSequencer *pSeq = (HnSequencer *)calloc(1, sizeof(HnSequencer));

    return pSeq;
}

void hn_sequencer_release(HnSequencer *pSeq)
{
    free(pSeq);
}

void hn_sequencer_attach(HnSequencer *pSeq, HnMixer *pMixer)
{
    pSeq->pMixer = pMixer;

    hn_mixer_add_stream(pMixer, pSeq, seq_internal_sync, 255);
    hn_mixer_add_stream(pMixer, pSeq, seq_internal_gen_click, 0);

    pSeq->pNextBlock = 
        (HnSequencerBlockState *)calloc(1, sizeof(HnSequencerBlockState));
}

void hn_sequencer_play(HnSequencer *pSeq)
{
    pSeq->pCommandedSignature = (HnTimeSignature *)calloc(1, sizeof(HnTimeSignature));
    pSeq->pCommandedSignature->jiffyTempo = 120.0f;
}

void hn_sequencer_set_bpm(HnSequencer *pSeq, float bpm)
{

}

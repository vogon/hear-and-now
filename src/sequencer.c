/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#include "hn.h"
#include "sequencer.h"

#include <stdlib.h>
#include <memory.h>

#define SAMPLE_RATE 44100

float *seq_internal_sync(void *pContext, uint64_t start, uint32_t length)
{
    HnSequencer *pSeq = (HnSequencer *)pContext;

    /* switch to any new time signature we got */
    if (pSeq->pSignature != pSeq->pCommandedSignature)
    {
        free(pSeq->pSignature);
        pSeq->pSignature = pSeq->pCommandedSignature;
    }

    /* if signature is null, we're not playing */
    if (NULL != pSeq->pSignature)
    {
        float samplesPerJiffy = (float)SAMPLE_RATE / pSeq->pSignature->jiffyTempo;
        jiffies_t jiffy;
        float sample;

        /* wake up everything every jiffy */
        for (jiffy = pSeq->pNextBlock->nextJiffy,
             sample = pSeq->pNextBlock->nextJiffyStart;
             sample < length;
             jiffy++, sample += samplesPerJiffy)
        {
            seq_internal_awaken_all(pSeq, jiffy, sample);
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

    pSeq->pNextBlock = 
        (HnSequencerBlockState *)calloc(1, sizeof(HnSequencerBlockState));
}

void hn_sequencer_play(HnSequencer *pSeq)
{
    seq_internal_set_tempo(pSeq, 120.0f);
}

void hn_sequencer_trigger(HnSequencer *pSeq, HnCmdQueue *pQueue, HnCmd *pCmd, 
    jiffies_t jiffy)
{
    HnTrigger *pTrig = (HnTrigger *)calloc(1, sizeof(HnTrigger));

    pTrig->jiffy = jiffy;
    pTrig->pQueue = pQueue;
    pTrig->pCmd = pCmd;
    pTrig->pNext = NULL;

    seq_internal_trigger_insert(pSeq, pTrig);
}

void seq_internal_set_tempo(HnSequencer *pSeq, float jiffyTempo)
{
    pSeq->pCommandedSignature = (HnTimeSignature *)calloc(1, sizeof(HnTimeSignature));
    pSeq->pCommandedSignature->jiffyTempo = jiffyTempo;
}

void seq_internal_trigger_insert(HnSequencer *pSeq, HnTrigger *pTrig)
{
    /* 
     * traverse trigger list, insert before all triggers on equal or greater
     * jiffies 
     */
    HnTrigger *pPrev = NULL, *pNext = pSeq->pFirstTrigger;

    while (pNext != NULL) 
    {
        if (pNext->jiffy >= pTrig->jiffy)
        {
            /* pNext comes after pTrig, insert pTrig here */
            pPrev->pNext = pTrig;
            break;
        }

        pPrev = pNext;
        pNext = pNext->pNext;
    }

    if (pPrev == NULL)
    {
        pSeq->pFirstTrigger = pTrig;
    }
}

void seq_internal_awaken_all(HnSequencer *pSeq, jiffies_t jiffy, uint32_t sample)
{
    /*
     * traverse trigger list, trigger all at <= jiffy
     */
    HnTrigger *pTrig = pSeq->pFirstTrigger;

    while (pTrig != NULL)
    {
        if (pTrig->jiffy <= jiffy)
        {
            HnTrigger *pNext = pTrig->pNext;

            /* update local sample count */
            pTrig->pCmd->sample = sample;

            /* send the message and then free the trigger */
            printf("boop (%d)\n", pTrig->pCmd->code);
            hn_cmd_queue_send(pTrig->pQueue, pTrig->pCmd);
            free(pTrig);

            /* move on */
            pTrig = pNext;
        }
        else
        {
            break;
        }
    }

    /*
     * pTrig is the first un-fired trigger. update the trigger list to point to it.
     */
    pSeq->pFirstTrigger = pTrig;
}
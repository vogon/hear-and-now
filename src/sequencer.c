/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#include <stdlib.h>

#include "hn.h"
#include "sequencer.h"

HnSequencer *hn_sequencer_create()
{
    HnSequencer *pSeq = (HnSequencer *)calloc(1, sizeof(HnSequencer));

    pSeq->bpm = 120.0f;

    return pSeq;
}

void hn_sequencer_release(HnSequencer *pSeq)
{
    free(pSeq);
}

void hn_sequencer_set_transport(HnSequencer *pSeq, HnMixer *pMixer)
{
    pSeq->pTransport = pMixer;
}

void hn_sequencer_set_bpm(HnSequencer *pSeq, float bpm)
{
    pSeq->bpm = bpm;
}

void hn_sequencer_play(HnSequencer *pSeq)
{
    // do magic here
}

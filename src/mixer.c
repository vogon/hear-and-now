/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "mixer.h"

#define BUFFER_LENGTH 2048

void mixer_internal_audio_cb(void *context, uint32_t pending)
{
    HnMixer_impl *pMixerImpl = (HnMixer_impl *)context;

    pMixerImpl->audioLowWater = (pending < 5);
    hn_cv_wake(pMixerImpl->pAudioLowWater);
}

HnMixer *hn_mixer_create(HnAudio *pAudio)
{
    HnMixer *pMixer = (HnMixer *)malloc(sizeof(HnMixer));
    HnMixer_impl *pMixerImpl = (HnMixer_impl *)malloc(sizeof(HnMixer_impl));

    pMixer->pImpl = pMixerImpl;

    pMixerImpl->samplesMixed = 0;

    pMixerImpl->pAudio = pAudio;
    pMixerImpl->audioLowWater = 1;
    pMixerImpl->pAudioLock = hn_mutex_create();
    pMixerImpl->pAudioLowWater = hn_cv_create();

    pMixerImpl->pFirstStream = pMixerImpl->pLastStream = NULL;
    pMixerImpl->pStreamLock = hn_mutex_create();

    hn_audio_watch(pAudio, mixer_internal_audio_cb, pMixerImpl);

    return pMixer;
}

void hn_mixer_release(HnMixer *pMixer)
{
    free(pMixer->pImpl);
    free(pMixer);
}

void hn_mixer_add_stream(HnMixer *pMixer, void *pContext,
                         HnGeneratorFn callback, uint8_t priority)
{
    HnMixer_impl *pImpl = pMixer->pImpl;

    Stream *pStream = (Stream *)malloc(sizeof(Stream));

    pStream->priority = priority;
    pStream->pContext = pContext;
    pStream->volume = 0.2f;
    pStream->callback = callback;
    pStream->pNext = pStream->pPrev = NULL;

    mixer_internal_add_stream(pImpl, pStream);
}

void hn_mixer_start(HnMixer *pMixer)
{
    HnMixer_impl *pImpl = (HnMixer_impl *)pMixer->pImpl;

    while (1)
    {
        float accum[BUFFER_LENGTH] = {0.0f};

        hn_mutex_lock(pImpl->pStreamLock);

        for (Stream *pStream = pImpl->pFirstStream;
             pStream != NULL; pStream = pStream->pNext)
        {
            /* get unscaled audio data */
            float *source = pStream->callback(pStream->pContext, 0, BUFFER_LENGTH);

            if (!source)
            {
                /* 
                 * allow generators to pass back a null sample block (treat as 
                 * silence.)  the sequencer will use this for a sync "generator", and
                 * smart generators could detect that they'll be silent for a whole
                 * block and save the alloc/memset/add/free.
                 */
                continue;
            }

            for (int i = 0; i < BUFFER_LENGTH; i++)
            {
                /* scale it down */
                accum[i] += pStream->volume * source[i];
            }

            free(source);
        }

        hn_mutex_unlock(pImpl->pStreamLock);

        /* mix these floats down to int8_t */
        uint8_t *intAccum = (uint8_t *)malloc(BUFFER_LENGTH);

        for (int i = 0; i < BUFFER_LENGTH; i++)
        {
            float scaled = (accum[i] + 1) * 127.5;

            if (scaled > 255)
            {
                /* clamp to [0, 1] because otherwise oh god */
                intAccum[i] = 255;
            }
            else if (scaled < 0)
            {
                intAccum[i] = 0;
            }
            else
            {
                intAccum[i] = scaled;
            }
        }

        // for (int i = 0; i < BUFFER_LENGTH; i++) 
        // {
        //     printf("%hhd, ", intAccum[i]);
        // }

        /* send it to threadless */
        hn_mutex_lock(pImpl->pAudioLock);

        hn_audio_write(pImpl->pAudio, intAccum, BUFFER_LENGTH);

        /* update sample accounting */
        pImpl->samplesMixed += BUFFER_LENGTH;

        /* if we've built up a few blocks of lead, go to sleep */
        if (hn_audio_samples_pending(pImpl->pAudio) > 5)
        {
            hn_cv_sleep(pImpl->pAudioLowWater, pImpl->pAudioLock);
        }

        hn_mutex_unlock(pImpl->pAudioLock);
    }
}

void mixer_internal_add_stream(HnMixer_impl *pMixerImp, Stream *pStream)
{
    hn_mutex_lock(pMixerImp->pStreamLock);

    Stream *pPrev = NULL, *pNext = pMixerImp->pFirstStream;

    /* find proper insertion point; cursor is pNext, previous is pPrev */
    while (pNext != NULL)
    {
        if (pNext->priority < pStream->priority)
        {
            /* 
             * stream at cursor is lower-prio than the new one,
             * insert before this stream 
             */
            break;
        }

        pPrev = pNext;
        pNext = pNext->pNext;
    }

    /* insert */

    if (NULL != pPrev)
    {
        /* fix up back links */
        pStream->pPrev = pPrev;
        pPrev->pNext = pStream;
    }
    else
    {
        /* pPrev == NULL -> pStream is first */
        pMixerImp->pFirstStream = pStream;
    }

    if (NULL != pNext)
    {
        /* fix up forward links */
        pStream->pNext = pNext;
        pNext->pPrev = pStream;
    }
    else
    {
        /* pNext == NULL -> pStream is last */
        pMixerImp->pLastStream = pStream;
    }

    hn_mutex_unlock(pMixerImp->pStreamLock);
}

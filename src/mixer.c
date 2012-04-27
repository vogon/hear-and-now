/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "mixer.h"

#define BUFFER_LENGTH 2048

void mixer_internal_append_stream(HnMixer_impl *, Stream *);

int gogogo = 0;

void mixer_internal_audio_cb(uint32_t pending)
{
    gogogo = pending < 5;
}

HnMixer *hn_mixer_create(HnAudio *pAudio)
{
    HnMixer *pMixer = (HnMixer *)malloc(sizeof(HnMixer));
    HnMixer_impl *pMixerImpl = (HnMixer_impl *)malloc(sizeof(HnMixer_impl));

    pMixer->pImpl = pMixerImpl;

    pMixerImpl->pAudio = pAudio;
    pMixerImpl->pFirstStream = pMixerImpl->pLastStream = NULL;

    pAudio->watch(pAudio, mixer_internal_audio_cb);

    return pMixer;
}

void hn_mixer_release(HnMixer *pMixer)
{
    free(pMixer->pImpl);
    free(pMixer);
}

void hn_mixer_add_stream(HnMixer *pMixer, void *pContext,
                         float *(*callback)(void *, uint32_t))
{
    HnMixer_impl *pImpl = pMixer->pImpl;

    Stream *pStream = (Stream *)malloc(sizeof(Stream));

    pStream->pContext = pContext;
    pStream->volume = 0.5f;
    pStream->callback = callback;
    pStream->pNext = pStream->pPrev = NULL;

    mixer_internal_append_stream(pImpl, pStream);
}

void hn_mixer_start(HnMixer *pMixer)
{
    HnMixer_impl *pImpl = (HnMixer_impl *)pMixer->pImpl;

    while (1)
    {
        float accum[BUFFER_LENGTH] = {0.0f};

        for (Stream *pStream = pImpl->pFirstStream;
             pStream != NULL; pStream = pStream->pNext)
        {
            printf(".");
            fflush(stdout);

            /* get unscaled audio data */
            float *source = pStream->callback(pStream->pContext, BUFFER_LENGTH);

            for (int i = 0; i < BUFFER_LENGTH; i++)
            {
                /* scale it down */
                accum[i] += pStream->volume * source[i];
            }

            free(source);
        }

        /* mix these floats down to int8_t */
        uint8_t *intAccum = (uint8_t *)malloc(BUFFER_LENGTH);

        for (int i = 0; i < BUFFER_LENGTH; i++)
        {
            if (accum[i] > 1)
            {
                /* clamp to [0, 1] because otherwise oh god */
                intAccum[i] = 255;
            }
            else
            {
                intAccum[i] = (uint8_t)(accum[i] * 255.);
            }
        }

/*        for (int i = 0; i < BUFFER_LENGTH; i++) 
        {
            printf("%hhd, ", intAccum[i]);
        }
*/
        /* send it to threadless */
        pImpl->pAudio->write(pImpl->pAudio, intAccum, BUFFER_LENGTH);

        while (!gogogo)
        {
            usleep(20000);
        }
        
    }
}

void mixer_internal_append_stream(HnMixer_impl *pMixerImp, Stream *pStream)
{
    Stream *pPrev = pMixerImp->pLastStream;

    if (NULL != pPrev)
    {
        pStream->pPrev = pPrev;
        pPrev->pNext = pStream;
    }

    if (NULL == pMixerImp->pFirstStream)
    {
        pMixerImp->pFirstStream = pStream;
    }

    pMixerImp->pLastStream = pStream;
}
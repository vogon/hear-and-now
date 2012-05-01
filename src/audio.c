/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#include <stdlib.h>

#include "hn.h"

#include "audio.h"
#include "win32/audio-win32.h"

HnAudio *hn_audio_open() 
{
    return hn_win32_audio_open();
}

void hn_audio_watch(HnAudio *pAudio, void (*callback)(void *, uint32_t), void *context)
{
    pAudio->pVtbl->watch(pAudio, callback, context);
}

void hn_audio_write(HnAudio *pAudio, uint8_t *pData, uint32_t len)
{
    pAudio->pVtbl->write(pAudio, pData, len);
}

uint32_t hn_audio_samples_pending(HnAudio *pAudio)
{
    return pAudio->pVtbl->samples_pending(pAudio);
}

void hn_audio_close(HnAudio *pAudio)
{
    pAudio->pVtbl->close(pAudio);
}

/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#include <stdio.h>

#include "audio.h"
#include "audio-win32.h"

typedef struct WAVEHDR_QueueTag
{
    WAVEHDR *pHdr;
    struct WAVEHDR_QueueTag *pNext;
} WAVEHDR_QueueTag;

typedef struct WatchCbQueueTag
{
    void (*callback)(void *, uint32_t);
    void *context;
    struct WatchCbQueueTag *pNext;
} WatchCbQueueTag;

typedef struct
{
    const void *pVtbl;

    HWAVEOUT hwo;
    uint32_t headersPending;

    WAVEHDR_QueueTag *pFirstUnprepare, *pLastUnprepare;
    WatchCbQueueTag *pFirstWatchCb, *pLastWatchCb;
} HnAudio_Win32;

const HnAudio_vtbl _HnAudio_Win32_vtbl =
    {
        hn_win32_audio_watch,
        hn_win32_audio_write,
        hn_win32_audio_samples_pending,
        hn_win32_audio_close,
    };

void queue_for_unprepare(HnAudio_Win32 *pAudioImpl, WAVEHDR *pHdr)
{
    WAVEHDR_QueueTag *pNew = (WAVEHDR_QueueTag *)malloc(sizeof(WAVEHDR_QueueTag));
    pNew->pHdr = pHdr;
    pNew->pNext = NULL;

    WAVEHDR_QueueTag *pPrev = pAudioImpl->pLastUnprepare;

    if (pPrev != NULL)
    {
        pPrev->pNext = pNew;
    }

    if (pAudioImpl->pFirstUnprepare == NULL)
    {
        pAudioImpl->pFirstUnprepare = pNew;
    }

    pAudioImpl->pLastUnprepare = pNew;
}

void unprepare_all(HnAudio_Win32 *pAudioImpl)
{
    for (WAVEHDR_QueueTag *pTag = pAudioImpl->pFirstUnprepare;
         pTag != NULL;)
    {
        waveOutUnprepareHeader(pAudioImpl->hwo, pTag->pHdr, sizeof(WAVEHDR));
        free(pTag->pHdr->lpData);
        free(pTag->pHdr);

        /* save aside next, free this one */
        WAVEHDR_QueueTag *pNext = pTag->pNext;
        free(pTag);
        pTag = pNext;
    }

    pAudioImpl->pFirstUnprepare = pAudioImpl->pLastUnprepare = NULL;
}

void enqueue_watch(HnAudio_Win32 *pAudioImpl, void (*callback)(void *, uint32_t), void *context)
{
    WatchCbQueueTag *pNew = (WatchCbQueueTag *)malloc(sizeof(WatchCbQueueTag));

    pNew->callback = callback;
    pNew->context = context;
    pNew->pNext = NULL;

    WatchCbQueueTag *pPrev = pAudioImpl->pLastWatchCb;

    if (pPrev != NULL)
    {
        pPrev->pNext = pNew;
    }

    if (pAudioImpl->pFirstWatchCb == NULL)
    {
        pAudioImpl->pFirstWatchCb = pNew;
    }

    pAudioImpl->pLastWatchCb = pNew;    
}

void signal_pending_all(HnAudio_Win32 *pAudioImpl)
{
    for (WatchCbQueueTag *pTag = pAudioImpl->pFirstWatchCb; 
         pTag != NULL; pTag = pTag->pNext)
    {
        pTag->callback(pTag->context, pAudioImpl->headersPending);
    }
}

void build_wfex(WAVEFORMATEX *pWfex, DWORD sampleRate, WORD sampleRes, WORD channels)
{
    memset(pWfex, 0, sizeof(WAVEFORMATEX));

    pWfex->wFormatTag = WAVE_FORMAT_PCM;
    pWfex->nChannels = channels;
    pWfex->nSamplesPerSec = sampleRate;
    pWfex->wBitsPerSample = sampleRes;

    pWfex->nBlockAlign = pWfex->nChannels * pWfex->wBitsPerSample / 8;
    pWfex->nAvgBytesPerSec = pWfex->nSamplesPerSec * pWfex->nBlockAlign;

    // cbSize is ignored
}

void CALLBACK wave_out_proc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, 
    DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    switch (uMsg)
    {
    case WOM_DONE:
    {
        WAVEHDR *pHdr = (WAVEHDR *)dwParam1;
        HnAudio_Win32 *pAudioImpl = (HnAudio_Win32 *)dwInstance;

        /* can't actually unprepare a buffer in this callback, 
           for fear of deadlock */
        queue_for_unprepare(pAudioImpl, pHdr);
        pAudioImpl->headersPending--;
        signal_pending_all(pAudioImpl);

        break;
    }
    default:
        break;
    }
}

HnAudio *hn_win32_audio_open(HnAudioFormat *pFormat)
{
    HnAudio_Win32 *pImpl = 
        (HnAudio_Win32 *)calloc(1, sizeof(HnAudio_Win32));
    
    WAVEFORMATEX wfex;
    build_wfex(&wfex, 
               pFormat->samplesPerSecond, 
               pFormat->sampleResolution, 
               pFormat->numberOfChannels);

    waveOutOpen(&(pImpl->hwo), 0, &wfex, (DWORD_PTR)wave_out_proc, 
        (DWORD_PTR)pImpl, CALLBACK_FUNCTION);

    pImpl->pVtbl = &_HnAudio_Win32_vtbl;
    return (HnAudio *)pImpl;
}

void hn_win32_audio_watch(HnAudio *pAudio, void (*callback)(void *, uint32_t), void *context)
{
    enqueue_watch((HnAudio_Win32 *)pAudio, callback, context);
}

void hn_win32_audio_write(HnAudio *pAudio, uint8_t *pData, uint32_t len)
{
    WAVEHDR *hdr = (WAVEHDR *)malloc(sizeof(WAVEHDR));
    HnAudio_Win32 *pImpl = (HnAudio_Win32 *)pAudio;

    /* before we enqueue another buffer, unprepare/free previously-played ones */
    unprepare_all(pImpl);

    hdr->lpData = (LPSTR)pData;
    hdr->dwBufferLength = len;
    hdr->dwFlags = 0;

    waveOutPrepareHeader(pImpl->hwo, hdr, sizeof(WAVEHDR));
    waveOutWrite(pImpl->hwo, hdr, sizeof(WAVEHDR));

    pImpl->headersPending++;
    signal_pending_all(pImpl);
}

uint32_t hn_win32_audio_samples_pending(HnAudio *pAudio)
{
    /* note: this code is currently totally busted.  I'll fix it eventually. */
    HnAudio_Win32 *pImpl = (HnAudio_Win32 *)pAudio;

    return pImpl->headersPending;
}

void hn_win32_audio_close(HnAudio *pAudio)
{
    HnAudio_Win32 *pImpl = (HnAudio_Win32 *)pAudio;

    waveOutClose(pImpl->hwo);

    free(pImpl);
}

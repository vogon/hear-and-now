/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#include "audio-win32.h"

typedef struct
{
    HWAVEOUT hwo;
} HnAudio_impl_Win32;

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

void hn_win32_audio_open(HnAudio *pAudio)
{
    HnAudio_impl_Win32 *pImpl = (HnAudio_impl_Win32 *)calloc(1, sizeof(HnAudio_impl_Win32));
    
    WAVEFORMATEX wfex;
    build_wfex(&wfex, 44100, 8, 1);

    waveOutOpen(&(pImpl->hwo), 0, &wfex, 0, 0, CALLBACK_NULL);

    pAudio->pImpl = pImpl;
    pAudio->close = hn_win32_audio_close;

    /* test code */
    CHAR buf[256];
    WAVEHDR hdr;

    {
        /* build crappy sawtooth */
        for (int i = 0; i < 256; i++) 
        {
            buf[i] = i - 128;
        }

        /* populate buffer header */
        hdr.lpData = buf;
        hdr.dwBufferLength = 256;
        hdr.dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
    }

    waveOutPrepareHeader(pImpl->hwo, &hdr, sizeof(hdr));
    waveOutWrite(pImpl->hwo, &hdr, sizeof(hdr));
}

void hn_win32_audio_close(HnAudio *pAudio)
{
    HnAudio_impl_Win32 *pImpl = (HnAudio_impl_Win32 *)pAudio->pImpl;

    waveOutClose(pImpl->hwo);

    free(pImpl);
    free(pAudio);
}

/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#include <stdlib.h>

#include "hn.h"

#include "win32/audio-win32.h"

HnAudio *hn_audio_open() 
{
    HnAudio *pAudio = (HnAudio *)calloc(1, sizeof(HnAudio));

    hn_win32_audio_open(pAudio);

    return pAudio;
}
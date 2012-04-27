/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#pragma once

#ifndef _HN_AUDIO_WIN32_H
#define _HN_AUDIO_WIN32_H

#include <windows.h>

#include "hn.h"

#ifdef __cplusplus
extern "C" {
#endif

void hn_win32_audio_open(HnAudio *pAudio);

void hn_win32_audio_watch(HnAudio *pAudio, void (*callback)(uint32_t));

void hn_win32_audio_write(HnAudio *pAudio, uint8_t *pData, uint32_t len);

void hn_win32_audio_close(HnAudio *pAudio);

#ifdef __cplusplus
}
#endif

#endif /* _HN_AUDIO_WIN32_H */
/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#pragma once

#ifndef _HN_SEQUENCER_H
#define _HN_SEQUENCER_H

#include <stdint.h>

#include "hn.h"

#ifdef __cplusplus
extern "C" {
#endif

struct HnSequencer
{
    HnMixer *pTransport;
    float bpm;
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _HN_SEQUENCER_H */

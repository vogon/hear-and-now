/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#pragma once

#ifndef _HN_12TET_H
#define _HN_12TET_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

const float A4_12TET = 440.0f;

extern inline float up_12tet(float root, uint8_t semitones)
{
    return root * powf(2.0f, (float)semitones / 12.);
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _HN_12TET_H */

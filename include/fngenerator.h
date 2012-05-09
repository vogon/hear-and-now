/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#pragma once

#ifndef _HN_FNGENERATOR_H
#define _HN_FNGENERATOR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef float (*HnFunction)(float theta);

struct HnFunctionGenerator;
typedef struct HnFunctionGenerator HnFunctionGenerator;

HnFunctionGenerator *hn_fngen_create(HnFunction function, float frequency, float theta0);
float *hn_fngen_generate(void *context, uint64_t start, uint32_t len);

float hn_square(float theta);
float hn_triangle(float theta);
float hn_saw(float theta);
float hn_sine(float theta);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _HN_FNGENERATOR_H */

/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#include "hn.h"
#include "fngenerator.h"

#include <stdlib.h>
#include <math.h>

struct HnFunctionGenerator
{
    HnFunction function;

    float nextTheta, thetaStep;
};

#define M_PI 3.14159265358979323846

#define SAMPLE_RATE 44100

HnFunctionGenerator *hn_fngen_create(HnFunction function, float frequency, float theta0)
{
    HnFunctionGenerator *pGenerator = 
        (HnFunctionGenerator *)calloc(1, sizeof(HnFunctionGenerator));

    pGenerator->function = function;
    pGenerator->thetaStep = 2.0f * M_PI * frequency / SAMPLE_RATE;
    pGenerator->nextTheta = theta0;

    return pGenerator;
}

float *hn_fngen_generate(void *context, uint64_t start, uint32_t len) 
{
    HnFunctionGenerator *state = (HnFunctionGenerator *)context;
    float *buf = (float *)malloc(len * sizeof(float));

    float theta = state->nextTheta;

    for (int i = 0; i < len; i++)
    {
        buf[i] = state->function(theta);

        theta = fmod(theta + state->thetaStep, 2.0f * M_PI);
    }

    state->nextTheta = theta;

    return buf;
}

float hn_square(float theta)
{
    return (theta - M_PI > 0) ? 1 : 0;
}

float hn_triangle(float theta)
{
    float f = fabs((theta - M_PI) / M_PI);

    // printf("%f\n", f);
    return f;
}

float hn_saw(float theta)
{
    return theta / (2 * M_PI);
}

float hn_sine(float theta)
{
    return (sin(theta) / 2) + 1.0f;
}
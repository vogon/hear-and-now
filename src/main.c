#ifdef WINDOWS
#include <windows.h>
#include <mmsystem.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "hn.h"

typedef struct Saw
{
    float step;
    float last;
    float *(*generate)(void *, uint32_t);
} Saw;

float *gen_sawtooth(void *context, uint32_t len) 
{
    Saw *state = (Saw *)context;
    float *buf = (float *)malloc(len * sizeof(float));

    float last = state->last;

    for (int i = 0; i < len; i++) 
    {
        float next = fmod(last + state->step, 1.0f);

        buf[i] = next;
        last = next;
    }

    state->last = last;

    return buf;
}

Saw *make_sawtooth(HnAudioFormat *pFormat, float frequency)
{
    Saw *result = (Saw *)malloc(sizeof(struct Saw));

    float samples_per_cycle = (float)pFormat->samplesPerSecond / frequency;

    result->step = 1.0f / samples_per_cycle;
    result->last = -result->step;
    result->generate = gen_sawtooth;

    // printf("make-sawtooth: %f, %f, %f\n", samples_per_cycle, result->step, result->last);

    return result;
}

typedef struct Square
{
    Saw *saw;
    float pwm;
    float *(*generate)(void *context, uint32_t len);
} Square;

float *gen_square(void *context, uint32_t len)
{
    Square *square = (Square *)context;
    Saw *saw = square->saw;

    float *buf = saw->generate(saw, len);

    for (int i = 0; i < len; i++)
    {
        float sample = buf[i];
        buf[i] = sample >= square->pwm ? ceil(sample) : floor(sample);
    }

    return buf;
}

Square *make_square(HnAudioFormat *pFormat, float frequency, float pwm)
{
    Square *result = (Square *)calloc(1, sizeof(Square));

    result->saw = make_sawtooth(pFormat, frequency);
    result->pwm = pwm;
    result->generate = gen_square;

    return result;
}

typedef struct Triangle
{
    Saw *saw;
    int flip;
    float *(*generate)(void *, uint32_t);
} Triangle;

float *gen_triangle(void *context, uint32_t len)
{
    Triangle *triangle = (Triangle *)context;
    Saw *saw = triangle->saw;

    float *buf = saw->generate(saw, len);

    float previous = saw->last;
    for (int i = 0; i < len; i++)
    {
        float current = buf[i];

        if (previous >= current)
        {
            triangle->flip = !triangle->flip;
        }

        if (triangle->flip)
        {
            buf[i] = 1 - current;
        }

        previous = current;
    }

    return buf;
}


Triangle *make_triangle(HnAudioFormat *pFormat, float frequency)
{
    Triangle *result = (Triangle *)malloc(sizeof(Triangle));

    result->saw = make_sawtooth(pFormat, frequency);
    result->flip = 0;
    result->generate = gen_triangle;

    return result;
}

float up(float root, uint8_t semitones)
{
    return root * powf(2.0f, (float)semitones / 12.);
}

int main() 
{
    HnAudioFormat fmt = { 44100, 8, 1 };

    float root = 220.0f;
    float third = up(root, 4);
    float fifth = up(root, 7);
    float octave = up(root, 11);
    float ninth = up(root, 14);

    Square *squares[5] = {
        make_square(&fmt, root, 0.5),
        make_square(&fmt, third, 0.45),
        make_square(&fmt, fifth, 0.4),
        make_square(&fmt, octave, 0.35),
        make_square(&fmt, ninth, 0.3)
    };

    // float *wave = gen_sawtooth(saw, 512);
    
    // for (int i = 0; i < 512; i++) 
    // {
    //     printf("%hhd, ", (int8_t)((wave[i] * 0.5f * (127 - -128)) - 128));
    // }

    HnAudio *audio = hn_audio_open(&fmt);
    HnMixer *mixer = hn_mixer_create(audio);

    for (int i = 0; i < 5; i++) {
        hn_mixer_add_stream(mixer, squares[i], squares[i]->generate);
    }

    hn_mixer_start(mixer);

    // audio->close(audio);

    return EXIT_SUCCESS;
}

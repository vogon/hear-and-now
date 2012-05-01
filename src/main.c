#ifdef WINDOWS
#include <windows.h>
#include <mmsystem.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "hn.h"

typedef struct Saw {
    float step;
    float last;
} Saw;

Saw *make_sawtooth(HnAudioFormat *pFormat, float frequency)
{
    Saw *result = (Saw *)malloc(sizeof(struct Saw));

    float samples_per_cycle = (float)pFormat->samplesPerSecond / frequency;

    result->step = 1.0f / samples_per_cycle;
    result->last = -result->step;

    // printf("make-sawtooth: %f, %f, %f\n", samples_per_cycle, result->step, result->last);

    return result;
}

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

typedef struct Square {
    Saw *saw;
    float pwm;
} Square;

Square *make_square(HnAudioFormat *pFormat, float frequency, float pwm)
{
    Square *result = (Square *)malloc(sizeof(Square));

    result->saw = make_sawtooth(pFormat, frequency);
    result->pwm = pwm;

    return result;
}

float *gen_square(void *context, uint32_t len)
{
    Square *square = (Square *)context;
    float *buf = gen_sawtooth(square->saw, len);
    for (int i = 0; i < len; i++) {
        float sample = buf[i];
        buf[i] = sample >= square->pwm ? ceil(sample) : floor(sample);
    }
    return buf;
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
    float octave = up(root, 12);

    Saw *saw = make_sawtooth(&fmt, root);
    Saw *saw2 = make_sawtooth(&fmt, third);
    Saw *saw3 = make_sawtooth(&fmt, fifth);
    Square *square = make_square(&fmt, octave, 0.5);

    // float *wave = gen_sawtooth(saw, 512);
    
    // for (int i = 0; i < 512; i++) 
    // {
    //     printf("%hhd, ", (int8_t)((wave[i] * 0.5f * (127 - -128)) - 128));
    // }

    HnAudio *audio = hn_audio_open(&fmt);
    HnMixer *mixer = hn_mixer_create(audio);

    hn_mixer_add_stream(mixer, saw, gen_sawtooth);
    hn_mixer_add_stream(mixer, saw2, gen_sawtooth);
    hn_mixer_add_stream(mixer, saw3, gen_sawtooth);
    hn_mixer_add_stream(mixer, square, gen_square);

    hn_mixer_start(mixer);

    // audio->close(audio);

    return EXIT_SUCCESS;
}

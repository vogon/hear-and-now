#include <windows.h>
#include <mmsystem.h>

#include <stdio.h>
#include <math.h>

#include "hn.h"

#define FULL_SCALE_LOW -128
#define FULL_SCALE_HIGH 127
#define SAMPLE_RATE 44100

typedef struct Saw {
    float step;
    float last;
} Saw;

Saw *make_sawtooth(float frequency)
{
    Saw *result = (Saw *)malloc(sizeof(struct Saw));

    float samples_per_cycle = (float)SAMPLE_RATE / frequency;

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

float up(float root, uint8_t semitones)
{
    return root * powf(2.0f, (float)semitones / 12.);
}

int main() 
{
    float root = 220.0f;
    float third = up(root, 4);
    float fifth = up(root, 7);

    Saw *saw = make_sawtooth(root);
    Saw *saw2 = make_sawtooth(third);
    Saw *saw3 = make_sawtooth(fifth);

    // float *wave = gen_sawtooth(saw, 512);
    
    // for (int i = 0; i < 512; i++) 
    // {
    //     printf("%hhd, ", (int8_t)((wave[i] * 0.5f * (127 - -128)) - 128));
    // }

    HnAudio *audio = hn_audio_open();
    HnMixer *mixer = hn_mixer_create(audio);

    hn_mixer_add_stream(mixer, saw, gen_sawtooth);
    hn_mixer_add_stream(mixer, saw2, gen_sawtooth);
    hn_mixer_add_stream(mixer, saw3, gen_sawtooth);

    hn_mixer_start(mixer);

    // audio->close(audio);

    return EXIT_SUCCESS;
}
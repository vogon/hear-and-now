#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "hn.h"
#include "12tet.h"
#include "fngenerator.h"

int main() 
{
    HnAudioFormat fmt = { 44100, 8, 1 };

    float root = A4_12TET;
    float third = up_12tet(root, 4);
    float fifth = up_12tet(root, 7);
    float octave = up_12tet(root, 11);
    float ninth = up_12tet(root, 14);

    HnFunctionGenerator *gens[5] = 
    {
        hn_fngen_create(hn_sine, root, 0),
        hn_fngen_create(hn_triangle, third, 0),
        hn_fngen_create(hn_square, fifth, 0),
        hn_fngen_create(hn_saw, octave, 0),
        hn_fngen_create(hn_saw, ninth, 0)
    };

    HnAudio *audio = hn_audio_open(&fmt);
    HnMixer *mixer = hn_mixer_create(audio);

    for (int i = 0; i < 5; i++)
    {
        hn_mixer_add_stream(mixer, gens[i], hn_fngen_generate, 0);
    }

    HnSequencer *seq = hn_sequencer_create();
    HnCmdQueue *q = hn_cmd_queue_create();
    HnNoteOnCmd cmd = { 0, CmdNoteOn, 440.0f };

    hn_sequencer_trigger(seq, q, (HnCmd *)&cmd, 64);

    hn_sequencer_attach(seq, mixer);
    hn_sequencer_play(seq);

    hn_mixer_start(mixer);

    // audio->close(audio);

    return EXIT_SUCCESS;
}

#include <windows.h>
#include <mmsystem.h>

#include "hn.h"

int main() 
{
    HnAudio *audio = hn_audio_open();

    while (1) {}

    audio->close(audio);

    return EXIT_SUCCESS;
}
#include "Audio.h"

void Audio::Play(Sound& sound, bool loop)
{
    if (loop) {
        if (!IsSoundPlaying(sound))
            PlaySound(sound);
    }
    else {
        PlaySound(sound);
    }
}

void Audio::Stop(Sound& sound)
{
    if (IsSoundPlaying(sound))
        StopSound(sound);
}

void Audio::Play(Music& music)
{
    if (!IsMusicStreamPlaying(music))
        PlayMusicStream(music);
}

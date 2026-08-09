#include "Audio.h"

void Audio::Play(Sound& sound, bool loop , bool condition)
{
	if (!loop)
	{
		if(!IsSoundPlaying(sound))
			PlaySound(sound);
	}
	else {
		if (condition) {
			if (!IsSoundPlaying(sound))
				PlaySound(sound);
		}
		else {
			if (IsSoundPlaying(sound))
				StopSound(sound);
		}
	}	
}

void Audio::Play(Music& music)
{
	if (!IsMusicStreamPlaying(music))
		PlayMusicStream(music);
}

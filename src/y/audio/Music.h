#ifndef AUDIO_MUSIC_H
#define AUDIO_MUSIC_H

#include "audio.h"

class Path;

namespace audio {


class Music {
public:
	float volume, speed;

	unsigned int al_source, al_buffer[2];
	AudioStream stream;

	Music();
	~Music();
	void __delete__();
	void play(bool loop);
	void set_rate(float rate);
	void stop();
	void pause(bool pause);
	bool is_playing();
	bool has_ended();

	void iterate();


	static Music* load(const Path &filename);
};

}

#endif //AUDIO_MUSIC_H

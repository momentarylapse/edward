#include "Music.h"
#include "SoundSource.h"
#include "audio.h"
#include "Loading.h"
#include "../y/EngineData.h"
#include "../lib/math/vec3.h"
#include "../lib/math/quaternion.h"
#include "../lib/os/file.h"
#include "../lib/os/msg.h"

#if HAS_LIB_OPENAL

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <al.h>
//#include <AL/alut.h>
#include <alc.h>

namespace audio {

//extern Array<Sound*> Sounds;
//extern Array<Music*> Musics;



bool AudioStream::stream(int buf) {
	if (state != AudioStream::State::READY)
		return false;
	load_stream_step(this);
	if (channels == 2) {
		if (bits == 8)
			alBufferData(buf, AL_FORMAT_STEREO8, buffer, buf_samples * 2, freq);
		else if (bits == 16)
			alBufferData(buf, AL_FORMAT_STEREO16, buffer, buf_samples * 4, freq);
	} else {
		if (bits == 8)
			alBufferData(buf, AL_FORMAT_MONO8, buffer, buf_samples, freq);
		else if (bits == 16)
			alBufferData(buf, AL_FORMAT_MONO16, buffer, buf_samples * 2, freq);
	}
	return true;
}

Music *Music::load(const Path &filename) {
	msg_write("loading sound " + str(filename));
	auto as = load_stream_start(engine.sound_dir | filename);

	Music *m = new Music();

	if (as.state == AudioStream::State::READY) {

		alGenSources(1, &m->al_source);
		alGenBuffers(2, m->al_buffer);
		m->stream = as;

		// start streaming
		int num_buffers = 0;
		if (as.stream(m->al_buffer[0]))
			num_buffers ++;
		if (as.stream(m->al_buffer[1]))
			num_buffers ++;
		alSourceQueueBuffers(m->al_source, num_buffers, m->al_buffer);


		alSourcef(m->al_source, AL_PITCH,           m->speed);
		alSourcef(m->al_source, AL_GAIN,            m->volume * VolumeMusic);
		alSourcei(m->al_source, AL_LOOPING,         false);
		alSourcei(m->al_source, AL_SOURCE_RELATIVE, AL_TRUE);
	}
	return m;
}

Music::Music() {
	volume = 1;
	speed = 1;
	al_source = 0;
	al_buffer[0] = 0;
	al_buffer[1] = 0;
}

Music::~Music() {
	stop();
	alSourceUnqueueBuffers(al_source, 2, al_buffer);
	load_stream_end(&stream);
	alDeleteBuffers(2, al_buffer);
	alDeleteSources(1, &al_source);
}

void Music::__delete__() {
	this->~Music();
}

void Music::play(bool loop) {
	//alSourcei   (al_source, AL_LOOPING, loop);
	alSourcePlay(al_source);
	alSourcef(al_source, AL_GAIN, volume * VolumeMusic);
}

void Music::set_rate(float rate) {
}

void Music::stop() {
	alSourceStop(al_source);
}

void Music::pause(bool pause) {
	int state;
	alGetSourcei(al_source, AL_SOURCE_STATE, &state);
	if (pause and (state == AL_PLAYING))
		alSourcePause(al_source);
	else if (!pause and (state == AL_PAUSED))
		alSourcePlay(al_source);
}

bool Music::is_playing() {
	int state;
	alGetSourcei(al_source, AL_SOURCE_STATE, &state);
	return (state == AL_PLAYING);
}

bool Music::has_ended() {
	return !is_playing();
}

void Music::iterate() {
	alSourcef(al_source, AL_GAIN, volume * VolumeMusic);
	int processed;
	alGetSourcei(al_source, AL_BUFFERS_PROCESSED, &processed);
	while (processed --) {
		ALuint buf;
		alSourceUnqueueBuffers(al_source, 1, &buf);
		if (stream.stream(buf))
			alSourceQueueBuffers(al_source, 1, &buf);
	}
}

}

#pragma GCC diagnostic pop

#else


namespace audio {

Music *Music::load(const Path &filename){ return nullptr; }
Music::~Music(){}
void Music::play(bool repeat){}
void Music::set_rate(float rate){}
void Music::stop(){}
void Music::pause(bool pause){}
bool Music::is_playing(){ return false; }
bool Music::has_ended(){ return false; }
void Music::iterate(){}

}

#endif


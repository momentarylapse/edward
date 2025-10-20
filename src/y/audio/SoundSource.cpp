#include "SoundSource.h"
#include "AudioBuffer.h"
#include "AudioStream.h"
#include "Loading.h"
#include <ecs/Entity.h>

#if HAS_LIB_OPENAL

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <al.h>
#include <alc.h>


namespace audio {


const kaba::Class *SoundSource::_class = nullptr;


SoundSource::SoundSource() {
	component_type = _class;
	suicidal = false;
	volume = 1;
	speed = 1;
	al_source = 0;
	loop = false;
	min_distance = 100;
	max_distance = 10000;

	alGenSources(1, &al_source);
}

SoundSource::~SoundSource() {
	stop();
	set_buffer(nullptr);
	//alDeleteBuffers(1, &al_buffer);
	alDeleteSources(1, &al_source);
}

void SoundSource::set_buffer(AudioBuffer* _buffer) {
	if (buffer) {
		buffer->ref_count --;
	}
	buffer = _buffer;
	if (buffer) {
		buffer->ref_count ++;
		alSourcei (al_source, AL_BUFFER, buffer->al_buffer);
	}
}

void SoundSource::set_stream(AudioStream* _stream) {
	if (buffer) {
		buffer->ref_count --;
	}
	if (stream) {

	}
	buffer = nullptr;
	stream = _stream;
	if (stream) {
		// start streaming
		int num_buffers = 0;
		if (stream->stream(stream->al_buffer[0]))
			num_buffers ++;
		if (stream->stream(stream->al_buffer[1]))
			num_buffers ++;
		alSourceQueueBuffers(al_source, num_buffers, stream->al_buffer);
	}
}

void SoundSource::_apply_data() {
	alSourcef (al_source, AL_PITCH,    speed);
	alSourcef (al_source, AL_GAIN,     volume * VolumeSound);
	alSource3f(al_source, AL_POSITION, -owner->pos.x, owner->pos.y, owner->pos.z);
//	alSource3f(al_source, AL_VELOCITY, -vel.x, vel.y, vel.z);
	alSourcei (al_source, AL_LOOPING,  loop);
	alSourcef (al_source, AL_REFERENCE_DISTANCE, min_distance);
	alSourcef (al_source, AL_MAX_DISTANCE, max_distance);
	alSourcef (al_source, AL_ROLLOFF_FACTOR, 1.0f);
}


void SoundSource::play(bool loop) {
	alSourcei(al_source, AL_LOOPING, loop);
	alSourcePlay(al_source);
}

void SoundSource::stop() {
	alSourceStop(al_source);
}

void SoundSource::pause(bool pause) {
	int state;
	alGetSourcei(al_source, AL_SOURCE_STATE, &state);
	if (pause and (state == AL_PLAYING))
		alSourcePause(al_source);
	else if (!pause and (state == AL_PAUSED))
		alSourcePlay(al_source);
}

bool SoundSource::is_playing() const {
	int state;
	alGetSourcei(al_source, AL_SOURCE_STATE, &state);
	return (state == AL_PLAYING);
}

bool SoundSource::has_ended() const {
	return !is_playing(); // TODO... (paused...)
}


}

#pragma GCC diagnostic pop

#else


namespace audio {

const kaba::Class *SoundSource::_class = nullptr;

xfer<SoundSource> load_sound(const Path &filename){ return nullptr; }
xfer<SoundSource> emit_sound(const Path &filename, const vec3 &pos, float min_dist, float max_dist, float speed, float volume, bool loop){ return nullptr; }
SoundSource::SoundSource() {}
SoundSource::~SoundSource() = default;
void SoundClearSmallCache() {}
void SoundSource::play(bool repeat) {}
void SoundSource::stop() {}
void SoundSource::pause(bool pause) {}
bool SoundSource::is_playing() const { return false; }
bool SoundSource::has_ended() const { return false; }
void SoundSource::set_buffer(AudioBuffer* _buffer) {}
void SoundSource::set_stream(AudioStream* _stream) {}
void SoundSource::_apply_data() {}

void clear_small_cache() {}

}

#endif

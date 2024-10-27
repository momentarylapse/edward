#include "audio.h"
#include "AudioBuffer.h"
#include "AudioStream.h"
#include "Listener.h"
#include "Loading.h"
#include "SoundSource.h"
#include "../helper/DeletionQueue.h"
#include "../world/World.h" // FIXME
#include "../y/ComponentManager.h"
#include "../y/Entity.h"
#include "../y/EngineData.h"
#include "../lib/base/base.h"
#include "../lib/base/algo.h"

#if HAS_LIB_OPENAL

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <al.h>
#include <alc.h>

#endif

namespace audio {

#if HAS_LIB_OPENAL
ALCdevice *al_dev = nullptr;
ALCcontext *al_context = nullptr;
#endif


void init() {
#if HAS_LIB_OPENAL
	al_dev = alcOpenDevice(nullptr);
	if (!al_dev)
		throw Exception("could not open openal device");
	al_context = alcCreateContext(al_dev, nullptr);
	if (!al_context)
			throw Exception("could not create openal context");
	alcMakeContextCurrent(al_context);
	alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
#endif
}

void exit() {
	reset();

#if HAS_LIB_OPENAL
	if (al_context)
		alcDestroyContext(al_context);
	al_context = nullptr;
	if (al_dev)
		alcCloseDevice(al_dev);
	al_dev = nullptr;
#endif
}

void attach_listener(Entity* e) {
	if (e)
		e->add_component<Listener>();
}

float VolumeMusic = 1.0f, VolumeSound = 1.0f;

void garbage_collection() {
	/*for (auto b: created_audio_buffers)
		if (b->ref_count <= 0) {
			delete b;
			b = nullptr;
		}
	for (auto&& [f, b]: loaded_audio_buffers)
		if (b->ref_count <= 0)
			b = nullptr;*/
}

void iterate(float dt) {
	auto& sources = ComponentManager::get_list<SoundSource>();
	for (auto s: sources) {
		// TODO owner->get_component<SolidBody>()->vel
		s->_apply_data();
		if (s->suicidal and s->has_ended()) {
			DeletionQueue::add(s->owner);

		} else if (s->stream) {
#if HAS_LIB_OPENAL
			int processed;
			alGetSourcei(s->al_source, AL_BUFFERS_PROCESSED, &processed);
			while (processed --) {
				ALuint buf;
				alSourceUnqueueBuffers(s->al_source, 1, &buf);
				if (s->stream->stream(buf))
					alSourceQueueBuffers(s->al_source, 1, &buf);
			}
#endif
		}
	}
	DeletionQueue::delete_all();
	auto& listeners = ComponentManager::get_list<Listener>();
	if (listeners.num >= 1)
		listeners[0]->apply_data();
}

void reset() {
}


SoundSource& emit_sound(AudioBuffer* buffer, const vec3 &pos, float radius1) {
	auto e = world.create_entity(pos, quaternion::ID);
	auto s = e->add_component<SoundSource>();
	s->set_buffer(buffer);
	s->min_distance = radius1;
	s->max_distance = radius1 * 100;
	s->suicidal = true;
	s->play(false);
	return *s;
}

SoundSource& emit_sound_file(const Path &filename, const vec3 &pos, float radius1) {
	return emit_sound(load_buffer(filename), pos, radius1);
}

SoundSource& emit_sound_stream(AudioStream* stream, const vec3 &pos, float radius1) {
	auto e = world.create_entity(pos, quaternion::ID);
	auto s = e->add_component<SoundSource>();
	s->set_stream(stream);
	s->min_distance = radius1;
	s->max_distance = radius1 * 100;
	s->suicidal = true;
	s->play(false);
	return *s;
}
}



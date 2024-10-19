#include "audio.h"

#include "Listener.h"
#include "Loading.h"
#include "SoundSource.h"
#include "../helper/DeletionQueue.h"
#include "../world/World.h" // FIXME
#include "../y/ComponentManager.h"
#include "../y/Entity.h"
#include "../lib/base/base.h"
#include "../lib/base/map.h"
#include "../lib/base/algo.h"
#include "../lib/math/vec3.h"
#include "../lib/os/path.h"

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

base::map<Path, AudioBuffer*> loaded_audio_buffers;
Array<AudioBuffer*> created_audio_buffers;

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
		if (s->suicidal and s->has_ended())
			DeletionQueue::add(s->owner);
	}
	DeletionQueue::delete_all();
	auto& listeners = ComponentManager::get_list<Listener>();
	if (listeners.num >= 1)
		listeners[0]->apply_data();
#if 0
	for (int i=Sounds.num-1;i>=0;i--)
		if (Sounds[i]->Suicidal)
			if (Sounds[i]->Ended())
				delete(Sounds[i]);
	for (int i=0;i<Musics.num;i++)
		Musics[i]->Iterate();
#endif
}

void reset() {
	/*for (int i=Sounds.num-1;i>=0;i--)
		delete(Sounds[i]);
	Sounds.clear();
	for (int i=Musics.num-1;i>=0;i--)
		delete(Musics[i]);
	Musics.clear();*/
}


AudioBuffer* load_buffer(const Path& filename) {
	int i = loaded_audio_buffers.find(filename);
	if (i >= 0)
		return loaded_audio_buffers.by_index(i);

	auto af = load_raw_buffer(filename);
	auto buffer = new AudioBuffer;

#if HAS_LIB_OPENAL
	alGenBuffers(1, &buffer->al_buffer);
	if (af.bits == 8)
		alBufferData(buffer->al_buffer, AL_FORMAT_MONO8, &af.buffer[0], af.samples, af.freq);
	else if (af.bits == 16)
		alBufferData(buffer->al_buffer, AL_FORMAT_MONO16, &af.buffer[0], af.samples * 2, af.freq);
#endif

	loaded_audio_buffers.set(filename, buffer);
	return buffer;
}

AudioBuffer* create_buffer(const Array<float>& samples, float sample_rate) {
	auto buffer = new AudioBuffer;

#if HAS_LIB_OPENAL
	alGenBuffers(1, &buffer->al_buffer);
	Array<short> buf16;
	buf16.resize(samples.num);
	for (int i=0; i<samples.num; i++)
		buf16[i] = (int)(samples[i] * 32768.0f);
	alBufferData(buffer->al_buffer, AL_FORMAT_MONO16, &buf16[0], samples.num * 2, (int)sample_rate);
#endif

	created_audio_buffers.add(buffer);
	return buffer;
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
}



#include "audio.h"
#include "AudioBuffer.h"
#include "AudioStream.h"
#include "Listener.h"
#include "Loading.h"
#include "SoundSource.h"
#include <helper/DeletionQueue.h>
#include <ecs/EntityManager.h>
#include <ecs/Entity.h>
#include <lib/base/base.h>
#include <lib/base/algo.h>
#include <world/components/Camera.h>

#if HAS_LIB_OPENAL

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <al.h>
#include <alc.h>

#endif

namespace audio {

const kaba::Class* Manager::_class = nullptr;

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
#if HAS_LIB_OPENAL
	if (al_context)
		alcDestroyContext(al_context);
	al_context = nullptr;
	if (al_dev)
		alcCloseDevice(al_dev);
	al_dev = nullptr;
#endif
}

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

Manager::Manager() {
	volume_sound = 1.0f;
	volume_music = 1.0f;
	set_profiler_name("audio");
}

void Manager::on_finished_loading() {
	entity_manager->add_component<Listener>(cam_main->owner);
}

void Manager::on_add_component(const ecs::MessageParams &params) {
	if (auto s = params.get<SoundSource>()) {
		s->_register();
	}
}

void Manager::on_remove_component(const ecs::MessageParams &params) {
	if (auto s = params.get<SoundSource>()) {
		s->unregister();
	}
}

void Manager::on_iterate(float dt) {
	auto& sources = entity_manager->get_component_list<SoundSource>();
	for (auto s: sources) {
		// TODO owner->get_component<RigidBody>()->vel
		s->_apply_data(volume_sound);
		if (s->suicidal and s->has_ended()) {
			DeletionQueue::add_entity(s->owner);

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

	auto& listeners = entity_manager->get_component_list<Listener>();
	if (listeners.num >= 1)
		listeners[0]->apply_data();
}

}



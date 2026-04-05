#pragma once

#include "audio.h"
#include <lib/math/vec3.h>
#include <lib/base/pointer.h>
#include <ecs/Component.h>

class Path;

namespace audio {

class Manager;

struct SoundSource : ecs::Component {
	bool loop, suicidal;
	float volume, speed;
	float min_distance, max_distance;

	AudioBuffer* buffer = nullptr;
	AudioStream* stream = nullptr;

	unsigned int al_source;

	SoundSource();

	void _register();
	void unregister();

	void set_buffer(AudioBuffer* buffer);
	void set_stream(AudioStream* stream);

	void play(bool loop);
	void stop();
	void pause(bool pause);
	bool is_playing() const;
	bool has_ended() const;

	void _apply_data(float global_volume);

	static const kaba::Class *_class;
};

}


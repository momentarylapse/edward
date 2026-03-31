#ifndef AUDIO_LISTENER_H
#define AUDIO_LISTENER_H

#include <ecs/Component.h>

namespace audio {

struct Listener : ecs::Component {
	Listener();
	void apply_data();

	static const kaba::Class *_class;
};

} // audio

#endif //AUDIO_LISTENER_H

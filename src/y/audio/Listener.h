#ifndef AUDIO_LISTENER_H
#define AUDIO_LISTENER_H

#include <ecs/Component.h>

namespace audio {

class Listener : public Component {
public:
	Listener();
	void apply_data();

	static const kaba::Class *_class;
};

} // audio

#endif //AUDIO_LISTENER_H

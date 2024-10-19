#ifndef AUDIO_LISTENER_H
#define AUDIO_LISTENER_H

#include "../y/Component.h"

namespace audio {

class Listener : public Component {
public:
	Listener();
	void apply_data();

	static const kaba::Class *_class;
};

} // audio

#endif //AUDIO_LISTENER_H

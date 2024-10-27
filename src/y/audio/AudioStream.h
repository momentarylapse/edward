#ifndef AUDIO_AUDIOSTREAM_H
#define AUDIO_AUDIOSTREAM_H

#include <lib/base/base.h>
#include <functional>

class Path;

namespace audio {

struct AudioStream {
	unsigned int al_buffer[2] = {0, 0};

	AudioStream();
	virtual ~AudioStream();
	virtual bool stream(unsigned int buf) = 0;
};

AudioStream* load_stream(const Path& filename);
AudioStream* create_stream(std::function<Array<float>(int)> f, float sample_rate);

}

#endif //AUDIO_AUDIOSTREAM_H

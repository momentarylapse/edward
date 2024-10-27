#ifndef AUDIO_AUDIOBUFFER_H
#define AUDIO_AUDIOBUFFER_H

#include "../lib/base/base.h"

class Path;

namespace audio {


struct RawAudioBuffer {
	int channels, bits, samples, sample_rate;
	bytes buffer;
};

struct AudioBuffer {
	unsigned int al_buffer = 0;
	int ref_count = 0;

	AudioBuffer();
	~AudioBuffer();

	void fill(const RawAudioBuffer& buf);
};

AudioBuffer* load_buffer(const Path& filename);
AudioBuffer* create_buffer(const Array<float>& samples, float sample_rate);

}

#endif //AUDIO_AUDIOBUFFER_H

#include "AudioStream.h"
#include "Loading.h"

#include "../lib/os/msg.h"

#if HAS_LIB_OPENAL

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <al.h>
#include <alc.h>

#endif

namespace audio {

void fill_buffer_f32(unsigned int buffer, const Array<float>& samples, float sample_rate);

AudioStream::AudioStream() {
#if HAS_LIB_OPENAL
	alGenBuffers(2, al_buffer);
#endif
}

AudioStream::~AudioStream() {
#if HAS_LIB_OPENAL
	alDeleteBuffers(2, al_buffer);
#endif
}

bool AudioStreamFile::stream(unsigned int buf) {
	if (state != AudioStreamFile::State::READY)
		return false;
#if HAS_LIB_OPENAL
	step();
	if (channels == 2) {
		if (bits == 8)
			alBufferData(buf, AL_FORMAT_STEREO8, &buffer[0], buf_samples * 2, freq);
		else if (bits == 16)
			alBufferData(buf, AL_FORMAT_STEREO16, &buffer[0], buf_samples * 4, freq);
	} else {
		if (bits == 8)
			alBufferData(buf, AL_FORMAT_MONO8, &buffer[0], buf_samples, freq);
		else if (bits == 16)
			alBufferData(buf, AL_FORMAT_MONO16, &buffer[0], buf_samples * 2, freq);
	}
#endif
	return true;
}

AudioStream* load_stream(const Path& filename) {
	return load_stream_start(filename);
}

struct AudioStreamUser : AudioStream {
	std::function<Array<float>(int)> f;
	float sample_rate = 44100;

	bool stream(unsigned int buf) override {
		const auto samples = f(65536);
		if (samples.num == 0)
			return false;
		fill_buffer_f32(buf, samples, sample_rate);
		return true;
	}
};

AudioStream* create_stream(std::function<Array<float>(int)> f, float sample_rate) {
	auto stream = new AudioStreamUser;
	stream->f = std::move(f);

	return stream;
}

}

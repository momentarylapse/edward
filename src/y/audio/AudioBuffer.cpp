#include "AudioBuffer.h"
#include "Loading.h"
#include "../lib/base/map.h"
#include "../lib/os/path.h"

#if HAS_LIB_OPENAL

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <al.h>
#include <alc.h>

#endif

namespace audio {

Array<AudioBuffer*> created_audio_buffers;
base::map<Path, AudioBuffer*> loaded_audio_buffers;

void fill_buffer_f32(unsigned int buffer, const Array<float>& samples, float sample_rate) {
#if HAS_LIB_OPENAL
	Array<short> buf16;
	buf16.resize(samples.num);
	for (int i = 0; i < samples.num; i++)
		buf16[i] = (short)(samples[i] * 32768.0f);
	alBufferData(buffer, AL_FORMAT_MONO16, &buf16[0], samples.num * 2, (int)sample_rate);
#endif
}

void fill_buffer_raw(unsigned int buffer, const RawAudioBuffer& raw) {
#if HAS_LIB_OPENAL
	if (raw.channels == 2) {
		if (raw.bits == 8)
			alBufferData(buffer, AL_FORMAT_STEREO8, raw.buffer.data, raw.buffer.num, raw.sample_rate);
		else if (raw.bits == 16)
			alBufferData(buffer, AL_FORMAT_STEREO16, raw.buffer.data, raw.buffer.num, raw.sample_rate);
	} else {
		if (raw.bits == 8)
			alBufferData(buffer, AL_FORMAT_MONO8, raw.buffer.data, raw.buffer.num, raw.sample_rate);
		else if (raw.bits == 16)
			alBufferData(buffer, AL_FORMAT_MONO16, raw.buffer.data, raw.buffer.num, raw.sample_rate);
	}
#endif
}

AudioBuffer::AudioBuffer() {
#if HAS_LIB_OPENAL
	alGenBuffers(1, &al_buffer);
#endif
}

AudioBuffer::~AudioBuffer() {
#if HAS_LIB_OPENAL
	alDeleteBuffers(1, &al_buffer);
#endif
}

void AudioBuffer::fill(const RawAudioBuffer& buf) {
	fill_buffer_raw(al_buffer, buf);
}


AudioBuffer* create_buffer(const Array<float>& samples, float sample_rate) {
	auto buffer = new AudioBuffer;
	fill_buffer_f32(buffer->al_buffer, samples, sample_rate);

	created_audio_buffers.add(buffer);
	return buffer;
}

AudioBuffer* load_buffer(const Path& filename) {
	int i = loaded_audio_buffers.find(filename);
	if (i >= 0)
		return loaded_audio_buffers.by_index(i);

	auto buffer = new AudioBuffer;
	buffer->fill(load_raw_buffer(filename));

	loaded_audio_buffers.set(filename, buffer);
	return buffer;
}
}

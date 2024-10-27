//
// Created by Michael Ankele on 2024-10-01.
//

#ifndef AUDIO_LOADING_H
#define AUDIO_LOADING_H

#include "audio.h"
#include "AudioStream.h"

class Path;

namespace audio {

struct RawAudioBuffer;


struct  AudioStreamFile : AudioStream {
	int channels, bits, samples, freq;
	bytes buffer;
	int buf_samples;

	enum class State {
		ERROR,
		READY,
		END
	} state;

	bool stream(unsigned int buf) override;
	virtual void step() = 0;
};


RawAudioBuffer load_raw_buffer(const Path& filename);
AudioStreamFile* load_stream_start(const Path& filename);


// writing
//void _cdecl SoundSaveFile(const string &filename, const Array<float> &data_r, const Array<float> &data_l, int freq, int channels, int bits);


}

#endif //AUDIO_LOADING_H

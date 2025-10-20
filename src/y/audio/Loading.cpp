#include "Loading.h"
#include "AudioBuffer.h"
#include "AudioStream.h"

#include <lib/os/filesystem.h>
#include <lib/os/file.h>
#include <lib/os/msg.h>
#include <EngineData.h>


#ifdef HAS_LIB_OGG
	#include <vorbis/codec.h>
	#include <vorbis/vorbisfile.h>
	#include <vorbis/vorbisenc.h>
#endif

namespace audio {


RawAudioBuffer EmptyAudioBuffer = {0, 0, 0, 0};

RawAudioBuffer load_wave_file(const Path &filename);
RawAudioBuffer load_ogg_file(const Path &filename);

RawAudioBuffer load_raw_buffer(const Path& filename) {
	msg_write("loading sound: " + filename.str());
	string ext = filename.extension();
	if (ext == "wav")
		return load_wave_file(engine.sound_dir | filename);
#ifdef HAS_LIB_OGG
	else if (ext == "ogg")
		return load_ogg_file(engine.sound_dir | filename);
#endif
	return EmptyAudioBuffer;
}


RawAudioBuffer load_wave_file(const Path &filename) {
	RawAudioBuffer r;
//	ProgressStatus(_("lade wave"), 0);
	auto f = os::fs::open(filename, "rb");
	r.buffer.resize(f->size());
	char header[44];
	f->read_basic(header, 44);
	if ((header[0] != 'R') or (header[1] != 'I') or (header[2] != 'F') or (header[3] != 'F')){
		msg_error("wave file does not start with \"RIFF\"");
		return r;
	}
	/*if (*(int*)&header[4] != f->GetSize())
		msg_write("wave file gives wrong size");
		// sometimes 0x2400ff7f*/
	if ((header[8] != 'W') or (header[9] != 'A') or (header[10] != 'V') or (header[11] != 'E') or (header[12] != 'f') or (header[13] != 'm') or (header[14] != 't') or (header[15] != ' ')){
		msg_error("\"WAVEfmt \" expected in wave file");
		return r;
	}
	if ((*(int*)&header[16] != 16) or (*(short*)&header[20] != 1)){
		msg_write("wave file does not have format 16/1");
		return r;
	}
	r.channels = *(short*)&header[22];
	r.sample_rate = *(int*)&header[24];
	[[maybe_unused]] int block_align = *(short*)&header[32];
	r.bits = *(short*)&header[34];
	int byte_per_sample = (r.bits / 8) * r.channels;
	if ((header[36] != 'd') or (header[37] != 'a') or (header[38] != 't') or (header[39] != 'a')){
		msg_error("\"data\" expected in wave file");
		return r;
	}
	int size = *(int*)&header[40];
	if ((size > f->size() - 44) or (size < 0)){
		msg_write("wave file gives wrong data size");
		size = f->size() - 44;
	}
	r.samples = size / byte_per_sample;
//	ProgressStatus(_("lade wave"), 0.1f);

	int read = 0;
	int nn = 0;
	while (read < size) {
		int toread = 65536;
		if (toread > size - read)
			toread = size - read;
		int rr = f->read_basic(&r.buffer[read], toread);
		nn ++;
/*		if (nn > 16){
			ProgressStatus(_("lade wave"), perc_read + dperc_read * (float)read / (float)size);
			nn = 0;
		}*/
		if (rr > 0) {
			read += rr;
		} else {
			msg_error("could not read in wave file...");
			break;
		}
	}

	delete f;

	return r;
}


#ifdef HAS_LIB_OGG

char ogg_buffer[4096];

RawAudioBuffer load_ogg_file(const Path &filename) {
	RawAudioBuffer r = EmptyAudioBuffer;
	OggVorbis_File vf;

	if (ov_fopen((char*)filename.c_str(), &vf)) {
		msg_error("ogg: ov_fopen failed");
		return r;
	}
	vorbis_info *vi = ov_info(&vf, -1);
	r.bits = 16;
	if (vi) {
		r.channels = vi->channels;
		r.sample_rate = vi->rate;
	}
	int bytes_per_sample = (r.bits / 8) * r.channels;
	r.samples = (int)ov_pcm_total(&vf, -1);
	char *data=new char[r.samples * bytes_per_sample + 4096];
	int current_section;
	int read = 0;
	while (true) {
		int toread = 4096;
		int rr = ov_read(&vf, &data[read], toread, 0, 2, 1, &current_section); // 0,2,1 = little endian, 16bit, signed
		if (rr == 0)
			break;
		if (rr < 0) {
			msg_error("ogg: ov_read failed");
			break;
		}
		read += rr;
	}
	ov_clear(&vf);
	r.samples = read / bytes_per_sample;
	r.buffer = data;
	return r;
}

struct AudioStreamOgg : AudioStreamFile {
	OggVorbis_File vf;

	explicit AudioStreamOgg(const Path& filename) {
		state = State::READY;
		buf_samples = 0;

		if (int res = ov_fopen((char*)filename.c_str(), &vf)) {
			state = State::ERROR;
			msg_error("ogg: ov_fopen failed: " + str(res));
			return;
		}
		vorbis_info *vi = ov_info(&vf, -1);
		bits = 16;
		if (vi) {
			channels = vi->channels;
			freq = vi->rate;
		}
		samples = (int)ov_pcm_total(&vf, -1);
		buffer.resize(65536 * 4 + 1024);
	}
	~AudioStreamOgg() override {
		ov_clear(&vf);
	}
	void step() override {
		if (state != State::READY)
			return;
		int current_section;
		int bytes_per_sample = (bits / 8) * channels;
		int wanted = 65536 * bytes_per_sample;

		int read = 0;
		while (read < wanted) {
			int toread = min(wanted - read, 4096);
			int rr = ov_read(&vf, (char*)&buffer[read], toread, 0, 2, 1, &current_section); // 0,2,1 = little endian, 16bit, signed
			if (rr == 0) {
				state = State::END;
				break;
			}
			if (rr < 0) {
				state = State::ERROR;
				msg_error("ogg: ov_read failed");
				break;
			}
			read += rr;
		}
		buf_samples = read / bytes_per_sample;
	}
};

#endif


AudioStreamFile* load_stream_start(const Path &filename) {
	string ext = filename.extension();
	/*if (ext == "wav")
		return load_wave_start(engine.sound_dir | filename);*/
#ifdef HAS_LIB_OGG
	if (ext == "ogg")
		return new AudioStreamOgg(engine.sound_dir | filename);
#endif
	return nullptr;
}

}

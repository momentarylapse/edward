//
// Created by Michael Ankele on 2024-10-01.
//

#ifndef AUDIO_AUDIO_H
#define AUDIO_AUDIO_H

#include "../lib/base/base.h"

struct vec3;
struct quaternion;
class Path;
class Entity;

namespace audio {

struct AudioBuffer;
struct AudioStream;

extern float VolumeMusic, VolumeSound;


void init();
void exit();
void attach_listener(Entity* e);
void iterate(float dt);
void reset();




class SoundSource;

// TODO move to World?
SoundSource& emit_sound(AudioBuffer* buffer, const vec3 &pos, float radius1);
SoundSource& emit_sound_file(const Path &filename, const vec3 &pos, float radius1);
SoundSource& emit_sound_stream(AudioStream* stream, const vec3 &pos, float radius1);

};



#endif //AUDIO_AUDIO_H

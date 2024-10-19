#include "Listener.h"
#include "../y/Entity.h"

#if HAS_LIB_OPENAL

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <al.h>
#include <alc.h>

#endif


namespace audio {

const kaba::Class *Listener::_class = nullptr;

Listener::Listener() {
	component_type = _class;
}

void Listener::apply_data() {
#if HAS_LIB_OPENAL
	ALfloat orientation[6];
	vec3 dir = owner->ang * vec3::EZ;
	orientation[0] = -dir.x;
	orientation[1] = dir.y;
	orientation[2] = dir.z;
	vec3 up = owner->ang * vec3::EY;
	orientation[3] = -up.x;
	orientation[4] = up.y;
	orientation[5] = up.z;
	alListener3f(AL_POSITION,    -owner->pos.x, owner->pos.y, owner->pos.z);
	//alListener3f(AL_VELOCITY,    -vel.x, vel.y, vel.z);
	alListenerfv(AL_ORIENTATION, orientation);
	//alSpeedOfSound(v_sound);
	alSpeedOfSound(100000);
#endif
}

} // audio


//
// Created by Michael Ankele on 2024-10-01.
//

#pragma once

#include <lib/base/base.h>
#include <ecs/System.h>

struct vec3;
struct quaternion;
class Path;
namespace ecs {
	struct Entity;
}

namespace audio {

struct AudioBuffer;
struct AudioStream;


void init();
void exit();


class Manager : public ecs::System {
public:
	Manager();
	void on_finished_loading() override;
	void on_iterate(float dt) override;

	void on_add_component(const ecs::MessageParams &params) override;
	void on_remove_component(const ecs::MessageParams &params) override;

	float volume_music, volume_sound;

	static const kaba::Class* _class;
};


struct SoundSource;

};


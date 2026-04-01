//
// Created by michi on 3/28/26.
//

#include "AnimationManager.h"
#include "../Model.h"
#include "../components/Animator.h"
#include "../components/Skeleton.h"
#include <lib/profiler/Profiler.h>

#include "lib/os/msg.h"

const kaba::Class* AnimationManager::_class = nullptr;

AnimationManager::AnimationManager() {
	set_profiler_name("ani");
}

void AnimationManager::on_iterate(float dt) {
	auto& list = entity_manager->get_component_list<Animator>();
	for (auto *o: list)
		o->do_animation(dt);


	// TODO
#if 0
	auto& list2 = entity_manager->get_component_list<Skeleton>();
	for (auto o: list2) {
		for (auto b: o->bones) {
			if ([[maybe_unused]] auto *mm = entity_get_model(b)) {
				//				b.dmatrix = matrix::translation(b.cur_pos) * matrix::rotation(b.cur_ang);
				//				mm->_matrix = o->get_owner<Entity3D>()->get_matrix() * b.dmatrix;
			}
		}
	}
#endif
}

void AnimationManager::on_add_component(const ecs::MessageParams &params) {
	if (auto a = params.get<Skeleton>()) {
		a->_register(entity_manager);
	} else if (auto a = params.get<Animator>()) {
		a->_register();
	}
}

void AnimationManager::on_remove_component(const ecs::MessageParams &params) {
	if (auto a = params.get<Skeleton>()) {
		a->unregister(entity_manager);
	} else if (auto a = params.get<Animator>()) {
		a->unregister();
	}
}

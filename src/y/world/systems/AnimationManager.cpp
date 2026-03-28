//
// Created by michi on 3/28/26.
//

#include "AnimationManager.h"
#include "../Model.h"
#include "../components/Animator.h"
#include "../components/Skeleton.h"
#include <lib/profiler/Profiler.h>

const kaba::Class* AnimationManager::_class = nullptr;

void AnimationManager::on_iterate(float dt) {
	profiler::begin(ch_iterate);
	auto& list = entity_manager->get_component_list<Animator>();
	for (auto *o: list)
		o->do_animation(dt);


	// TODO
	auto& list2 = entity_manager->get_component_list<Skeleton>();
	for (auto o: list2) {
		for (auto b: o->bones) {
			if ([[maybe_unused]] auto *mm = entity_get_model(b)) {
				//				b.dmatrix = matrix::translation(b.cur_pos) * matrix::rotation(b.cur_ang);
				//				mm->_matrix = o->get_owner<Entity3D>()->get_matrix() * b.dmatrix;
			}
		}
	}
	//o->do_animation(dt);
	profiler::end(ch_iterate);
}

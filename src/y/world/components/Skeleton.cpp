/*
 * Skeleton.cpp
 *
 *  Created on: Jul 17, 2021
 *      Author: michi
 */

#include "Skeleton.h"
#include "Animator.h"
#include "../Model.h"
#include "../../helper/ResourceManager.h"
#include "../../y/EngineData.h"
#include "../ModelManager.h"
#include "../../y/Entity.h"

const kaba::Class *Skeleton::_class = nullptr;

Skeleton::Skeleton() {
}

Skeleton::~Skeleton() {

	if (Model::AllowDeleteRecursive) {
		// delete sub models
//		for (auto &b: bone)
//			if (b.get_component<Model>())
//				delete b.get_component<Model>();
		// automatically when deleting bone entities
	}
}

void Skeleton::on_init() {
	auto m = owner->get_component<Model>();

	bones = m->_template->skeleton->bones;
	parents = m->_template->skeleton->parents;
	dpos = m->_template->skeleton->dpos;
	pos0 = m->_template->skeleton->pos0;

	// skeleton
	for (int i=0; i<bones.num; i++) {
		bones[i].parent = owner;
		pos0[i] = _calc_bone_rest_pos(i);
		bones[i].pos = pos0[i];
		bones[i].ang = quaternion::ID;
		auto mm = engine.resource_manager->load_model(m->_template->skeleton->filename[i]);
		if (mm) {
			bones[i]._add_component_external_(mm);

			if (mm->_template->skeleton)
				bones[i].add_component(Skeleton::_class, "");

			if (mm->_template->animator)
				bones[i].add_component(Animator::_class, "");
		}
		bones[i].on_init_rec();
	}
}

void Skeleton::on_delete() {
	for (auto &b: bones)
		b.on_delete_rec();
}

void Skeleton::reset() {
	for (int i=0; i<bones.num; i++){
		bones[i].ang = quaternion::ID;
		bones[i].pos = pos0[i];
	}
}

// non-animated state
vec3 Skeleton::_calc_bone_rest_pos(int index) const {
	if (parents[index] >= 0)
		return dpos[index] + _calc_bone_rest_pos(parents[index]);
	return dpos[index];
}


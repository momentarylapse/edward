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
#include "y/EntityManager.h"
#include <lib/base/iter.h>

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
	if (!m)
		return;

	// FIXME ...everything...

	for (const auto b: m->_template->skeleton->bones) {
		auto bb = EntityManager::global->create_entity(b->pos, b->ang);
		bb->parent = owner;
		bones.add(bb);
	}
	parents = m->_template->skeleton->parents;
	dpos = m->_template->skeleton->dpos;
	pos0 = m->_template->skeleton->pos0;

	// skeleton
	for (auto&& [i, b]: enumerate(bones)) {
		b->parent = owner;
		pos0[i] = _calc_bone_rest_pos(i);
		b->pos = pos0[i];
		b->ang = quaternion::ID;
		auto mm = engine.resource_manager->load_model(m->_template->skeleton->filename[i]);
		if (mm) {
			EntityManager::global->_add_component_external_(b, mm);

			if (mm->_template->skeleton)
				EntityManager::global->add_component<Skeleton>(b);

			if (mm->_template->animator)
				EntityManager::global->add_component<Animator>(b);
		}
	}
}

void Skeleton::on_delete() {
	for (auto b: bones)
		EntityManager::global->delete_entity(b);
	bones.clear();
}

void Skeleton::reset() {
	for (int i=0; i<bones.num; i++){
		bones[i]->ang = quaternion::ID;
		bones[i]->pos = pos0[i];
	}
}

// non-animated state
vec3 Skeleton::_calc_bone_rest_pos(int index) const {
	if (parents[index] >= 0)
		return dpos[index] + _calc_bone_rest_pos(parents[index]);
	return dpos[index];
}


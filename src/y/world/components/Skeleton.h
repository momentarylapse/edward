/*
 * Skeleton.h
 *
 *  Created on: Jul 17, 2021
 *      Author: michi
 */

#pragma once

#include "../../y/Component.h"
#include "../../y/Entity.h"

class Model;
class Path;


class Skeleton : public Component {
public:
	Skeleton();
	~Skeleton() override;

	void on_init() override;
	void on_delete() override;

	Array<Entity> bones; // pos relative to parent entity (skeleton)
	Array<int> parents;
	Array<vec3> pos0; // relative to parent entity (skeleton)
	Array<vec3> dpos; // relative to parent bone
	Array<Path> filename;

	void reset();

	vec3 _calc_bone_rest_pos(int index) const;
	void _cdecl set_bone_model(int index, Model *sub);


	static const kaba::Class *_class;
};

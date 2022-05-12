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
	virtual ~Skeleton();

	void on_init() override;
	void on_delete() override;

	Array<Entity> bones; // pos relative to parent entity (skeleton)
	Array<int> parents;
	Array<vector> pos0; // relative to parent entity (skeleton)
	Array<vector> dpos; // relative to parent bone
	Array<Path> filename;

	void reset();

	vector _calc_bone_rest_pos(int index) const;
	void _cdecl set_bone_model(int index, Model *sub);


	static const kaba::Class *_class;
};

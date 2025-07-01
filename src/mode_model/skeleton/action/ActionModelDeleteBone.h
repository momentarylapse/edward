/*
 * ActionModelDeleteBone.h
 *
 *  Created on: 19.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELDELETEBONE_H_
#define ACTIONMODELDELETEBONE_H_

#include <action/Action.h>
#include <lib/math/vec3.h>
#include <lib/math/vec4.h>

class ActionModelDeleteBone: public Action {
public:
	explicit ActionModelDeleteBone(int _index);
	string name() override { return "ModelDeleteBone"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	int index;
	Array<int> child;
	vec3 pos;
	int parent;
	Path filename;
	void *model;
	Array<vec3> move_dpos;
	Array<vec3> move_ang;
	Array<int> vertex;
	Array<ivec4> vertex_bone;
	Array<vec4> vertex_bone_weight;
};

#endif /* ACTIONMODELDELETEBONE_H_ */

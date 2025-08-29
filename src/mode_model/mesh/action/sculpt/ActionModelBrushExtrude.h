/*
 * ActionModelBrushExtrude.h
 *
 *  Created on: 22.08.2013
 *      Author: michi
 */

#ifndef ACTIONMODELBRUSHEXTRUDE_H_
#define ACTIONMODELBRUSHEXTRUDE_H_

#include <action/Action.h>
#include <lib/mesh/Polygon.h>
#include <lib/math/vec3.h>

class DataModel;

class ActionModelBrushExtrude: public Action {
public:
	ActionModelBrushExtrude(const vec3& pos, const vec3& n, float radius, float depth);
	string name() override { return "ModelBrushExtrude"; }

	void *execute(Data* d) override;
	void undo(Data* d) override;

private:
	vec3 pos, n;
	float radius, depth;
	Array<int> index;
	Array<vec3> pos_old;
};

#endif /* ACTIONMODELBRUSHEXTRUDE_H_ */

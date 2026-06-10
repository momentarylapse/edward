/*
 * ActionModelBrushExtrude.h
 *
 *  Created on: 22.08.2013
 *      Author: michi
 */

#ifndef ACTIONMODELBRUSHEXTRUDE_H_
#define ACTIONMODELBRUSHEXTRUDE_H_

#include <lib/history/Action.h>
#include <lib/polymesh/Polygon.h>
#include <lib/math/vec3.h>

class DataModel;

class ActionModelBrushExtrude: public history::Action {
public:
	ActionModelBrushExtrude(const vec3& pos, const vec3& n, float radius, float depth);
	string name() const override { return "ModelBrushExtrude"; }

	void *execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	vec3 pos, n;
	float radius, depth;
	Array<int> index;
	Array<vec3> pos_old;
};

#endif /* ACTIONMODELBRUSHEXTRUDE_H_ */

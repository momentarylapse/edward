/*
 * ActionModelPolygonAddVertex.h
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#ifndef ACTIONMODELPOLYGONADDVERTEX_H_
#define ACTIONMODELPOLYGONADDVERTEX_H_

#include "../../../../../lib/math/vec3.h"
#include "../../../../ActionGroup.h"
#include <y/world/Material.h>

class ActionModelPolygonAddVertex: public ActionGroup {
public:
	ActionModelPolygonAddVertex(int _poly, int _side, int _vertex, const vec3 *_sv);
	string name() override { return "ModelPolygonAddVertex"; }

	void *compose(Data *d) override;

private:
	int poly;
	int side;
	int vertex;
	vec3 sv[MATERIAL_MAX_TEXTURES];
};

#endif /* ACTIONMODELPOLYGONADDVERTEX_H_ */

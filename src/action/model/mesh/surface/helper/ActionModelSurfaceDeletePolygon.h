/*
 * ActionModelSurfaceDeletePolygon.h
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSURFACEDELETEPOLYGON_H_
#define ACTIONMODELSURFACEDELETEPOLYGON_H_

#include "../../../../Action.h"
class vec3;

class ActionModelSurfaceDeletePolygon: public Action {
public:
	ActionModelSurfaceDeletePolygon(int _index);
	string name() override { return "ModelSurfaceDeletePolygon"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	int index;
	int material;
	Array<int> vertex;
	Array<vec3> skin;
};

#endif /* ACTIONMODELSURFACEDELETEPOLYGON_H_ */

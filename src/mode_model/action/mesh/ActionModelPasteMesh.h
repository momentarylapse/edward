/*
 * ActionModelPasteGeometry.h
 *
 *  Created on: 07.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELPASTEGEOMETRY_H_
#define ACTIONMODELPASTEGEOMETRY_H_

#include <action/Action.h>
#include <data/mesh/PolygonMesh.h>
class DataModel;

class ActionModelPasteMesh : public Action {
public:
	ActionModelPasteMesh(const PolygonMesh &geo, int material);
	string name() override { return "ModelPasteMesh"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;
private:
	PolygonMesh geo;
	int default_material;
};

#endif /* ACTIONMODELPASTEGEOMETRY_H_ */

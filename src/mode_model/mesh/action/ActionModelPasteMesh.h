/*
 * ActionModelPasteGeometry.h
 *
 *  Created on: 07.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELPASTEGEOMETRY_H_
#define ACTIONMODELPASTEGEOMETRY_H_

#include <action/Action.h>
#include <lib/mesh/PolygonMesh.h>
struct ModelMesh;

class ActionModelPasteMesh : public Action {
public:
	ActionModelPasteMesh(ModelMesh* m, const PolygonMesh &geo, int material);
	string name() override { return "ModelPasteMesh"; }

	void *execute(Data* d) override;
	void undo(Data* d) override;
private:
	ModelMesh* mesh;
	PolygonMesh geo;
	int default_material;
};

#endif /* ACTIONMODELPASTEGEOMETRY_H_ */

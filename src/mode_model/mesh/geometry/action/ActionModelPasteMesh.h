/*
 * ActionModelPasteGeometry.h
 *
 *  Created on: 07.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELPASTEGEOMETRY_H_
#define ACTIONMODELPASTEGEOMETRY_H_

#include <lib/history/Action.h>
#include <lib/polymesh/Mesh.h>
struct ModelMesh;

class ActionModelPasteMesh : public history::Action {
public:
	ActionModelPasteMesh(ModelMesh* m, const polymesh::Mesh &geo, int material);
	string name() const override { return "ModelPasteMesh"; }

	void *execute(history::Data* d) override;
	void undo(history::Data* d) override;
private:
	ModelMesh* mesh;
	polymesh::Mesh geo;
	int default_material;
};

#endif /* ACTIONMODELPASTEGEOMETRY_H_ */

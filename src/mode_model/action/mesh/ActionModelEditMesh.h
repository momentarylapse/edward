//
// Created by Michael Ankele on 2025-05-18.
//

#ifndef ACTIONMODELEDITMESH_H
#define ACTIONMODELEDITMESH_H


#include <action/Action.h>
#include <data/mesh/PolygonMesh.h>
#include <data/mesh/MeshEdit.h>

struct ModelMesh;

class ActionModelEditMesh : public Action {
public:
	ActionModelEditMesh(ModelMesh* mesh, const MeshEdit& edit);
	string name() override { return "ModelEditMesh"; }

	void *execute(Data* d) override;
	void undo(Data* d) override;
private:
	ModelMesh* mesh;
	MeshEdit edit;
};



#endif //ACTIONMODELEDITMESH_H

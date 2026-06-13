//
// Created by Michael Ankele on 2025-05-18.
//

#ifndef ACTIONMODELEDITMESH_H
#define ACTIONMODELEDITMESH_H


#include <lib/history/Action.h>
#include <lib/polymesh/PolygonMesh.h>
#include <lib/polymesh/MeshEdit.h>

struct ModelMesh;

class ActionModelEditMesh : public history::Action {
public:
	ActionModelEditMesh(ModelMesh* mesh, const MeshEdit& edit);
	string name() const override { return "ModelEditMesh"; }

	void *execute(history::Data* d) override;
	void undo(history::Data* d) override;
private:
	ModelMesh* mesh;
	MeshEdit edit;
};



#endif //ACTIONMODELEDITMESH_H

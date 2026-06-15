//
// Created by Michael Ankele on 2025-02-19.
//

#ifndef ACTIONMODELDELETESELECTION_H
#define ACTIONMODELDELETESELECTION_H


#include <lib/history/Action.h>
#include <lib/polymesh/MeshEdit.h>
#include <stuff/Selection.h>

class DataModel;
struct ModelMesh;

class ActionModelDeleteSelection : public history::Action {
public:
	ActionModelDeleteSelection(ModelMesh* m, const Selection& sel, bool greedy);
	string name() const override { return "ModelDeleteSelection"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;
private:
	ModelMesh* mesh;
	polymesh::MeshEdit edit;
};


#endif //ACTIONMODELDELETESELECTION_H

//
// Created by Michael Ankele on 2025-02-20.
//

#ifndef ACTIONMODELMOVESELECTION_H
#define ACTIONMODELMOVESELECTION_H


#include <action/ActionMultiView.h>
#include <lib/base/set.h>

struct ModelMesh;
struct vec3;
enum class MultiViewType;

class ActionModelMoveSelection: public ActionMultiView {
public:
	explicit ActionModelMoveSelection(ModelMesh* m, const Data::Selection& selection);
	string name() override { return "ModelMoveSelection"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;
private:
	ModelMesh* mesh;
	Array<MultiViewType> type;
};



#endif //ACTIONMODELMOVESELECTION_H

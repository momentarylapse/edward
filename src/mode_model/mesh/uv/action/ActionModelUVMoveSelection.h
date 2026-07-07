//
// Created by michi on 6/10/26.
//

#ifndef EDWARD_ACTIONMODELUVMOVESELECTION_H
#define EDWARD_ACTIONMODELUVMOVESELECTION_H



#include <action/ActionMultiView.h>
#include <lib/base/set.h>
#include <view/Selection.h>

struct ModelMesh;
struct vec3;
enum class MultiViewType;

class ActionModelUVMoveSelection: public ActionMultiView {
public:
	explicit ActionModelUVMoveSelection(ModelMesh* m, const Selection& selection);
	string name() const override { return "ModelUVMoveSelection"; }

	void *execute(history::Data* d) override;
	void undo(history::Data* d) override;
private:
	ModelMesh* mesh;
	Array<int> polygon_no;
};


#endif //EDWARD_ACTIONMODELUVMOVESELECTION_H

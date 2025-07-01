//
// Created by Michael Ankele on 2025-07-01.
//

#ifndef ACTIONMODELMOVEBONES_H
#define ACTIONMODELMOVEBONES_H


#include <action/ActionMultiView.h>
#include <lib/base/set.h>

class DataModel;
struct vec3;
enum class MultiViewType;

class ActionModelMoveBones: public ActionMultiView {
public:
	explicit ActionModelMoveBones(DataModel* m, const Data::Selection& selection);
	string name() override { return "ModelMoveBones"; }

	void* execute(Data *d) override;
	void undo(Data *d) override;
private:
};



#endif //ACTIONMODELMOVEBONES_H

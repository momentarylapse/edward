/*
 * ActionWorldMoveSelection.h
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#ifndef ACTIONWORLDMOVESELECTION_H_
#define ACTIONWORLDMOVESELECTION_H_

#include <action/ActionMultiView.h>
#include <lib/base/set.h>
#include <view/Selection.h>

class DataWorld;
struct vec3;
enum class MultiViewType;

class ActionWorldMoveSelection: public ActionMultiView {
public:
	explicit ActionWorldMoveSelection(DataWorld *d, const Selection& selection);
	string name() const override { return "WorldMoveSelection"; }

	void *execute(history::Data* d) override;
	void undo(history::Data* d) override;
private:
	Array<MultiViewType> type;
	Array<quaternion> old_ang;
};

#endif /* ACTIONWORLDMOVESELECTION_H_ */

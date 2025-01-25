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

class DataWorld;
class vec3;
enum class MultiViewType;

class ActionWorldMoveSelection: public ActionMultiView {
public:
	explicit ActionWorldMoveSelection(DataWorld *d, const Data::Selection& selection);
	string name() override { return "WorldMoveSelection"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;
private:
	Array<MultiViewType> type;
};

#endif /* ACTIONWORLDMOVESELECTION_H_ */

/*
 * ActionWorldMoveSelection.h
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#ifndef ACTIONWORLDMOVESELECTION_H_
#define ACTIONWORLDMOVESELECTION_H_

#include "../ActionMultiView.h"
class DataWorld;
class vec3;

class ActionWorldMoveSelection: public ActionMultiView {
public:
	ActionWorldMoveSelection(DataWorld *d);
	string name() override { return "WorldMoveSelection"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;
private:
	Array<int> type;
};

#endif /* ACTIONWORLDMOVESELECTION_H_ */

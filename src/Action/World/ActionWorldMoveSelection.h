/*
 * ActionWorldMoveSelection.h
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#ifndef ACTIONWORLDMOVESELECTION_H_
#define ACTIONWORLDMOVESELECTION_H_

#include "../ActionMultiView.h"
#include "../../Data/Data.h"
#include "../../lib/file/file.h"
#include "../../lib/types/types.h"

class ActionWorldMoveSelection: public ActionMultiView
{
public:
	ActionWorldMoveSelection(Data *d, const vector &_pos0);
	virtual ~ActionWorldMoveSelection();
	string name(){	return "WorldMoveSelection";	}

	void *execute(Data *d);
	void undo(Data *d);
private:
	Array<int> terrain_index;
	Array<vector> terrain_old_data;
};

#endif /* ACTIONWORLDMOVESELECTION_H_ */

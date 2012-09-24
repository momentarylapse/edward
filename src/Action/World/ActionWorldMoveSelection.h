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
class vector;

class ActionWorldMoveSelection: public ActionMultiView
{
public:
	ActionWorldMoveSelection(DataWorld *d, const vector &_param, const vector &_pos0);
	virtual ~ActionWorldMoveSelection();
	string name(){	return "WorldMoveSelection";	}

	void *execute(Data *d);
	void undo(Data *d);
private:
	Array<int> terrain_index;
	Array<vector> terrain_old_data;
};

#endif /* ACTIONWORLDMOVESELECTION_H_ */

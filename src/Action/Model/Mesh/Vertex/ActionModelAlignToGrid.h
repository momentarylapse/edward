/*
 * ActionModelAlignToGrid.h
 *
 *  Created on: 10.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELALIGNTOGRID_H_
#define ACTIONMODELALIGNTOGRID_H_

#include "../../../Action.h"
#include "../../../../Data/Model/DataModel.h"

class ActionModelAlignToGrid : public Action
{
public:
	ActionModelAlignToGrid(DataModel *m, float _grid_dist);
	virtual ~ActionModelAlignToGrid();
	string name(){	return "ModelNearifyVertices";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	Array<int> index;
	Array<vector> old_pos;
	float grid_dist;
};

#endif /* ACTIONMODELALIGNTOGRID_H_ */

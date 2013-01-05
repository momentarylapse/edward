/*
 * ActionModelMergePolygonsSelection.h
 *
 *  Created on: 05.01.2013
 *      Author: michi
 */

#ifndef ACTIONMODELMERGESELECTIONSELECTION_H_
#define ACTIONMODELMERGESELECTIONSELECTION_H_

#include "../../../ActionGroup.h"
class DataModel;
class ModelSurface;

class ActionModelMergePolygonsSelection: public ActionGroup
{
public:
	ActionModelMergePolygonsSelection();
	virtual ~ActionModelMergePolygonsSelection(){}
	string name(){	return "ModelMergePolygonsSelection";	}

	void *compose(Data *d);
private:
	void MergePolygonsInSurface(DataModel *m, ModelSurface *s, int surface);
	void MergePolygons(DataModel *m, ModelSurface *s, int surface, int edge);
};

#endif /* ACTIONMODELMERGESELECTIONSELECTION_H_ */

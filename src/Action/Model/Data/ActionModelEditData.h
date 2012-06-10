/*
 * ActionModelEditData.h
 *
 *  Created on: 10.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELEDITDATA_H_
#define ACTIONMODELEDITDATA_H_

#include "../../Action.h"
#include "../../../Data/Model/DataModel.h"

class ActionModelEditData : public Action
{
public:
	ActionModelEditData(const DataModel::MetaData &_data);
	virtual ~ActionModelEditData();
	string name(){	return "ModelEditData";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	DataModel::MetaData data;
};

#endif /* ACTIONMODELEDITDATA_H_ */

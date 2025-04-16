/*
 * ActionModelEditData.h
 *
 *  Created on: 10.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELEDITDATA_H_
#define ACTIONMODELEDITDATA_H_

#include <action/Action.h>
#include "../../data/DataModel.h"

class ActionModelEditData : public Action {
public:
	explicit ActionModelEditData(const DataModel::MetaData &_data);
	string name() override { return "ModelEditData"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	DataModel::MetaData data;
};

#endif /* ACTIONMODELEDITDATA_H_ */

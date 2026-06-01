/*
 * ActionModelEditData.h
 *
 *  Created on: 10.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELEDITDATA_H_
#define ACTIONMODELEDITDATA_H_

#include <lib/history/Action.h>
#include "../data/DataModel.h"

class ActionModelEditData : public history::Action {
public:
	explicit ActionModelEditData(const DataModel::MetaData& _data);
	string name() const override { return "ModelEditData"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	DataModel::MetaData data;
};

#endif /* ACTIONMODELEDITDATA_H_ */

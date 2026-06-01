/*
 * ActionWorldEditData.h
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>
#include "../data/DataWorld.h"

class ActionWorldEditData : public history::Action {
public:
	explicit ActionWorldEditData(const DataWorld::MetaData &_data);
	string name() const override { return "WorldEditData"; }

	void *execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	DataWorld::MetaData data;
};


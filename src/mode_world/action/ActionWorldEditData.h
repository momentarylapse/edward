/*
 * ActionWorldEditData.h
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#ifndef ACTIONWORLDEDITDATA_H_
#define ACTIONWORLDEDITDATA_H_

#include <action/Action.h>
#include "../data/DataWorld.h"

class ActionWorldEditData : public Action {
public:
	explicit ActionWorldEditData(const DataWorld::MetaData &_data);
	string name() override { return "WorldEditData"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	DataWorld::MetaData data;
};

#endif /* ACTIONWORLDEDITDATA_H_ */

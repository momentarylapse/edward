/*
 * ActionWorldEditData.h
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#ifndef ACTIONWORLDEDITDATA_H_
#define ACTIONWORLDEDITDATA_H_

#include "../Action.h"
#include "../../Data/World/DataWorld.h"

class ActionWorldEditData : public Action
{
public:
	ActionWorldEditData(const DataWorld::MetaData &_data);
	virtual ~ActionWorldEditData();
	string name(){	return "WorldEditData";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	DataWorld::MetaData data;
};

#endif /* ACTIONWORLDEDITDATA_H_ */

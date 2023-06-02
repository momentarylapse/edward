/*
 * ActionWorldSetEgo.h
 *
 *  Created on: 15.06.2012
 *      Author: michi
 */

#ifndef ACTIONWORLDSETEGO_H_
#define ACTIONWORLDSETEGO_H_

#include "../Action.h"

class ActionWorldSetEgo : public Action
{
public:
	ActionWorldSetEgo(int _index);
	virtual ~ActionWorldSetEgo();
	string name(){	return "WorldSetEgo";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int index;
};

#endif /* ACTIONWORLDSETEGO_H_ */

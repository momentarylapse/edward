/*
 * ActionWorldPaste.h
 *
 *  Created on: 25.09.2012
 *      Author: michi
 */

#ifndef ACTIONWORLDPASTE_H_
#define ACTIONWORLDPASTE_H_

#include "../../action/ActionGroup.h"

class DataWorld;
struct WorldEntity;

class ActionWorldPaste : public ActionGroup {
public:
	explicit ActionWorldPaste(const DataWorld& temp);
	string name() override { return "WorldPaste"; }

	void *compose(Data *d) override;
private:
	const Array<WorldEntity>& entities;
};

#endif /* ACTIONWORLDPASTE_H_ */

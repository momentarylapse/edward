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
class WorldObject;
class WorldTerrain;
class WorldLight;
class WorldCamera;

class ActionWorldPaste : public ActionGroup {
public:
	explicit ActionWorldPaste(const DataWorld& temp);
	string name() override { return "WorldPaste"; }

	void *compose(Data *d) override;
private:
	const Array<WorldObject>& objects;
	const Array<WorldTerrain>& terrains;
	const Array<WorldCamera>& cameras;
	const Array<WorldLight>& lights;
};

#endif /* ACTIONWORLDPASTE_H_ */

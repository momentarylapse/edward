/*
 * ActionWorldRotateObjects.h
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#ifndef ACTIONWORLDROTATEOBJECTS_H_
#define ACTIONWORLDROTATEOBJECTS_H_

#include "../../ActionMultiView.h"
class DataWorld;
class vec3;

class ActionWorldRotateObjects: public ActionMultiView {
public:
	ActionWorldRotateObjects(DataWorld *d);
	string name() override { return "WorldRotateObjects"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;
private:
	Array<vec3> old_ang;
	Array<int> type;
};

#endif /* ACTIONWORLDROTATEOBJECTS_H_ */

/*
 * ActionModelSurfaceAutoWeld.h
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSURFACEAUTOWELD_H_
#define ACTIONMODELSURFACEAUTOWELD_H_

#include "../../../ActionGroup.h"
class DataModel;

class ActionModelSurfaceAutoWeld : public ActionGroup
{
public:
	ActionModelSurfaceAutoWeld(int _surface1, int _surface2, float _epsilon);
	virtual ~ActionModelSurfaceAutoWeld(){}
	string name(){	return "ModelSurfaceAutoWeld";	}

	void *compose(Data *d);
private:
	int surface1, surface2;
	float epsilon;
};

#endif /* ACTIONMODELSURFACEAUTOWELD_H_ */

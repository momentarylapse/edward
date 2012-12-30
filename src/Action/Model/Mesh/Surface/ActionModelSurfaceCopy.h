/*
 * ActionModelSurfaceCopy.h
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSURFACECOPY_H_
#define ACTIONMODELSURFACECOPY_H_

#include "../../../ActionGroup.h"
class DataModel;
class ModelSurface;

class ActionModelSurfaceCopy : public ActionGroup
{
public:
	ActionModelSurfaceCopy(int surface);
	virtual ~ActionModelSurfaceCopy();
	string name(){	return "ModelSurfaceCopy";	}

	void *compose(Data *d);
private:
	int surface;
};

#endif /* ACTIONMODELSURFACECOPY_H_ */

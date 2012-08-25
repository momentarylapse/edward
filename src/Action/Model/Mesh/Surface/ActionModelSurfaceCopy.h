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
class ModeModelSurface;

class ActionModelSurfaceCopy : public ActionGroup
{
public:
	ActionModelSurfaceCopy(DataModel *m, ModeModelSurface *&s);
	virtual ~ActionModelSurfaceCopy();
	string name(){	return "ModelSurfaceCopy";	}

	void *execute_return(Data *d);
};

#endif /* ACTIONMODELSURFACECOPY_H_ */

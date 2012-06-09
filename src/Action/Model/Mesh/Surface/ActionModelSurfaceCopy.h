/*
 * ActionModelSurfaceCopy.h
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSURFACECOPY_H_
#define ACTIONMODELSURFACECOPY_H_

#include "../../../Action.h"
#include "../../../ActionGroup.h"
#include "../../../../lib/types/types.h"
#include "../../../../Data/Model/DataModel.h"

class ActionModelSurfaceCopy : public ActionGroup
{
public:
	ActionModelSurfaceCopy(DataModel *m, ModeModelSurface *&s);
	virtual ~ActionModelSurfaceCopy();

	void *execute_return(Data *d);
};

#endif /* ACTIONMODELSURFACECOPY_H_ */

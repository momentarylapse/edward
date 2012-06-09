/*
 * ActionModelAddTriangleSingleTexture.h
 *
 *  Created on: 01.04.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDTRIANGLESINGLETEXTURE_H_
#define ACTIONMODELADDTRIANGLESINGLETEXTURE_H_

#include "../../../ActionGroup.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../lib/types/types.h"

class ActionModelAddTriangleSingleTexture: public ActionGroup
{
public:
	ActionModelAddTriangleSingleTexture(DataModel *m, int _a, int _b, int _c, int _material, const vector &_sva, const vector &_svb, const vector &_svc);
	virtual ~ActionModelAddTriangleSingleTexture();

	void *execute_return(Data *d);

private:
	void *dummy;
};

#endif /* ACTIONMODELADDTRIANGLESINGLETEXTURE_H_ */

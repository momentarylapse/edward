/*
 * ActionModelAddTriangleSingleTexture.cpp
 *
 *  Created on: 01.04.2012
 *      Author: michi
 */

#include "ActionModelAddTriangleSingleTexture.h"
#include "ActionModelAddTriangle.h"

ActionModelAddTriangleSingleTexture::ActionModelAddTriangleSingleTexture(DataModel *m, int _a, int _b, int _c, int _material, const vector &_sva, const vector &_svb, const vector &_svc)
{
	vector sv[3][MODEL_MAX_TEXTURES];
	for (int l=0;l<MODEL_MAX_TEXTURES;l++){
		sv[0][l] = _sva;
		sv[1][l] = _svb;
		sv[2][l] = _svc;
	}

	dummy = AddSubAction(new ActionModelAddTriangle(m, _a, _b, _c, _material, sv[0], sv[1], sv[2]), m);
}

ActionModelAddTriangleSingleTexture::~ActionModelAddTriangleSingleTexture()
{
}


void *ActionModelAddTriangleSingleTexture::execute_return(Data *d)
{
	return dummy;
}

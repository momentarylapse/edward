/*
 * ActionModelAddTriangleWithSkinGenerator.cpp
 *
 *  Created on: 21.08.2012
 *      Author: michi
 */

#include "ActionModelAddTriangleWithSkinGenerator.h"
#include "ActionModelAddTriangle.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../Data/Model/SkinGenerator.h"

ActionModelAddTriangleWithSkinGenerator::ActionModelAddTriangleWithSkinGenerator(DataModel *m, int _a, int _b, int _c, int _material, const SkinGenerator &sg)
{
	vector sv[3][MODEL_MAX_TEXTURES];
	for (int l=0;l<MODEL_MAX_TEXTURES;l++){
		sv[0][l] = sg.get(m->Vertex[_a].pos);
		sv[1][l] = sg.get(m->Vertex[_b].pos);
		sv[2][l] = sg.get(m->Vertex[_c].pos);
	}

	dummy = AddSubAction(new ActionModelAddTriangle(m, _a, _b, _c, _material, sv[0], sv[1], sv[2]), m);
}

ActionModelAddTriangleWithSkinGenerator::~ActionModelAddTriangleWithSkinGenerator()
{
}


void *ActionModelAddTriangleWithSkinGenerator::execute_return(Data *d)
{
	return dummy;
}

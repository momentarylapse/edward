/*
 * ModeModelMeshCreateDodecahedron.h
 *
 *  Created on: 09.01.2013
 *      Author: michi
 */

#ifndef MODEMODELMESHCREATEDODECAHEDRON_H_
#define MODEMODELMESHCREATEDODECAHEDRON_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class ModeModelMeshCreateDodecahedron: public ModeCreation
{
public:
	ModeModelMeshCreateDodecahedron(Mode *_parent);
	virtual ~ModeModelMeshCreateDodecahedron();

	void OnMouseMove();
	void OnLeftButtonDown();

	void OnDrawWin(int win);

	DataModel *data;
	Data *GetData(){	return data;	}

private:
	vector pos;
	float radius;
	bool pos_chosen;
};

#endif /* MODEMODELMESHCREATEDODECAHEDRON_H_ */

/*
 * ActionAddTriangle.cpp
 *
 *  Created on: 05.03.2012
 *      Author: michi
 */

#include "ActionModelAddTriangle.h"
#include "../../Data/Model/DataModel.h"
#include "ActionModel__AddSurface.h"
#include "ActionModel__SurfaceAddTriangle.h"
#include "ActionModel__JoinSurfaces.h"
#include <assert.h>

ActionModelAddTriangle::ActionModelAddTriangle(DataModel *m, int _a, int _b, int _c, int _material, const vector *_sva, const vector *_svb, const vector *_svc)
{
	assert(_a >= 0 && _b >= 0 && _c >= 0);
	assert(_a != _b && _b != _c && _c != _a);
	Array<int> v;
	v.add(_a);
	v.add(_b);
	v.add(_c);
	Set<int> surf;
	foreach(v, vv){
		if (m->Vertex[vv].Surface >= 0)
			surf.add(m->Vertex[vv].Surface);
	}

	if (surf.num == 0){
		// new surface
		AddSubAction(new ActionModel__AddSurface(), m);
		surf_no = m->Surface.num - 1;
	}else{
		// main surface?
		surf_no = surf[0];

		// join other surfaces into surf_no
		for (int i=surf.num-1;i>0;i--)
			AddSubAction(new ActionModel__JoinSurfaces(surf_no, surf[i]), m);
	}

	// add triangle
	AddSubAction(new ActionModel__SurfaceAddTriangle(surf_no, _a, _b, _c, _material, _sva, _svb, _svc), m);

	/*sModeModelSubSkin *sub = &skin->Sub[CurrentMaterial];
	sModeModelTriangle t;
	t.Index[0] = a;
	t.Index[1] = b;
	t.Index[2] = c;
	int v[3];
	v[0] = a;
	v[1] = b;
	v[2] = c;
	for (int tl=0;tl<sub->NumTextures;tl++){

		// any skin vertices attached to the vertices? (same material)
		vector sv[3] = {v0, e_x, e_y};
		for (int i=0;i<sub->Triangle.num;i++)
			if (sub->Triangle[i].ViewStage >= skin->ViewStage)
				for (int k=0;k<9;k++)
					if (sub->Triangle[i].Index[k/3] == v[k%3])
						sv[k%3] = sub->Triangle[i].SkinVertex[tl][k/3];

		t.SkinVertex[tl][0] = sv[0];
		t.SkinVertex[tl][1] = sv[1];
		t.SkinVertex[tl][2] = sv[2];
	}
	t.IsSelected = false;
	t.IsSpecial = false;
	t.ViewStage = skin->ViewStage;
	t.NormalDirty = true;
	sub->Triangle.add(t);*/
}

ActionModelAddTriangle::~ActionModelAddTriangle()
{
}


void *ActionModelAddTriangle::execute_return(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	return &m->Surface[surf_no].Triangle.back();
}

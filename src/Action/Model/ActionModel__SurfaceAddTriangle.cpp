/*
 * ActionModel__SurfaceAddTriangle.cpp
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#include "ActionModel__SurfaceAddTriangle.h"
#include "../../Data/Model/DataModel.h"

// might create a "disjoint" surface -> don't use alone!

ActionModel__SurfaceAddTriangle::ActionModel__SurfaceAddTriangle(int _surface, int _a, int _b, int _c, int _material, const vector *_sva, const vector *_svb, const vector *_svc)
{
	surface = _surface;
	a = _a;
	b = _b;
	c = _c;
	material = _material;
	for (int l=0;l<MODEL_MAX_TEXTURES;l++){
		sv[0][l] = _sva[l];
		sv[1][l] = _svb[l];
		sv[2][l] = _svc[l];
	}
}

ActionModel__SurfaceAddTriangle::~ActionModel__SurfaceAddTriangle()
{
}

void ActionModel__SurfaceAddTriangle::undo(Data *d)
{
	msg_write("__surf add tria undo");
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModeModelSurface *s = &m->Surface[surface];
	s->Triangle.pop();
	s->BuildFromTriangles();
}



void ActionModel__SurfaceAddTriangle::redo(Data *d)
{	execute(d);	}



void *ActionModel__SurfaceAddTriangle::execute(Data *d)
{
	msg_write("__surf add tria do");
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModeModelSurface *s = &m->Surface[surface];

	// add triangle
	s->AddTriangle(a, b, c, material, sv[0], sv[1], sv[2]);
	return &s->Triangle.back();

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



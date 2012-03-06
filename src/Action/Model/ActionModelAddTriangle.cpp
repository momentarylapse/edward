/*
 * ActionAddTriangle.cpp
 *
 *  Created on: 05.03.2012
 *      Author: michi
 */

#include "ActionModelAddTriangle.h"
#include "../../Data/Model/DataModel.h"

ActionModelAddTriangle::ActionModelAddTriangle(int _a, int _b, int _c, const vector &_sva, const vector &_svb, const vector &_svc)
{
	a = _a;
	b = _b;
	c = _c;
	sv[0] = _sva;
	sv[1] = _svb;
	sv[2] = _svc;
}

ActionModelAddTriangle::~ActionModelAddTriangle()
{
}

bool ActionModelAddTriangle::needs_preparation()
{	return false;	}



void *ActionModelAddTriangle::execute(Data *d)
{
	msg_write("add tria do");
	DataModel *m = dynamic_cast<DataModel*>(d);

	ModeModelTriangle *r;

	Array<int> v;
	v.add(a);
	v.add(b);
	v.add(c);
	Set<int> surf;
	foreach(v, vv){
		if (m->Vertex[vv].Surface >= 0)
			surf.add(m->Vertex[vv].Surface);
	}

	if (surf.num == 0){
		// new surface
		ModeModelSurface *s = m->AddSurface();

		s->AddTriangle(a, b, c, sv[0], sv[1], sv[2], m);
		r = &s->Triangle.back();
	}else{
		// main surface?
		ModeModelSurface *s = &m->Surface[surf[0]];

		// join surfaces
		for (int i=1;i<surf.num;i++)
			m->SurfaceJoin(s, &m->Surface[surf[i]]);

		// add triangle
		s->AddTriangle(a, b, c, sv[0], sv[1], sv[2], m);
		r = &s->Triangle.back();
	}

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

	return r;
}



void ActionModelAddTriangle::undo(Data *d)
{
	msg_write("add tria undo");
	DataModel *m = dynamic_cast<DataModel*>(d);

	foreach(m->Surface, s)
		foreachi(s.Triangle, t, i)
			if ((t.Vertex[0] == a) and (t.Vertex[1] == b) and (t.Vertex[2] == c)){
				m->Vertex[a].RefCount --;
				m->Vertex[b].RefCount --;
				m->Vertex[c].RefCount --;
				s.Triangle.erase(i);
			}
	msg_todo("ActionModelAddTriangle.undo...");
}



void ActionModelAddTriangle::prepare(Data *d)
{
}



void ActionModelAddTriangle::redo(Data *d)
{
	execute(d);
}




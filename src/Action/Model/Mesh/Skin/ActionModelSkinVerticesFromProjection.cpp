/*
 * ActionModelSkinVerticesFromProjection.cpp
 *
 *  Created on: 10.06.2012
 *      Author: michi
 */

#include "ActionModelSkinVerticesFromProjection.h"
#include <assert.h>

ActionModelSkinVerticesFromProjection::ActionModelSkinVerticesFromProjection(DataModel *m, MultiView *mv)
{
	irect r = mv->view[mv->mouse_win].dest;
	vector bg_a = vector((float)r.x1, (float)r.y1, 0);
	vector bg_b = vector((float)r.x2, (float)r.y2, 0);
	foreachi(m->Surface, s, si)
		foreachi(s.Triangle, t, ti)
			for (int k=0;k<3;k++)
				if (m->Vertex[t.Vertex[k]].is_selected){
					surface.add(si);
					triangle.add(ti);
					vert_on_tria.add(k);
					for (int l=0;l<MODEL_MAX_TEXTURES;l++)
						old_pos[l].add(t.SkinVertex[l][k]);
					vector v = mv->VecProject(m->Vertex[t.Vertex[k]].pos, mv->mouse_win);
					v.x = (v.x - bg_a.x) / (bg_b.x - bg_a.x);
					v.y = (v.y - bg_a.y) / (bg_b.y - bg_a.y);
					v.z = 0;
					new_pos.add(v);
				}
}

ActionModelSkinVerticesFromProjection::~ActionModelSkinVerticesFromProjection()
{
}

void *ActionModelSkinVerticesFromProjection::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(surface.num == triangle.num);
	assert(surface.num == vert_on_tria.num);
	assert(surface.num == old_pos[0].num);
	assert(surface.num == new_pos.num);

	foreachi(surface, s, i)
		for (int l=0;l<MODEL_MAX_TEXTURES;l++)
			m->Surface[s].Triangle[triangle[i]].SkinVertex[l][vert_on_tria[i]] = new_pos[i];

	return NULL;
}



void ActionModelSkinVerticesFromProjection::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(surface.num == triangle.num);
	assert(surface.num == vert_on_tria.num);
	assert(surface.num == old_pos[0].num);
	assert(surface.num == new_pos.num);

	foreachi(surface, s, i)
		for (int l=0;l<MODEL_MAX_TEXTURES;l++)
			m->Surface[s].Triangle[triangle[i]].SkinVertex[l][vert_on_tria[i]] = old_pos[l][i];
}



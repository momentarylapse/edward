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
	sg.init_projective(mv, mv->mouse_win);
}

ActionModelSkinVerticesFromProjection::~ActionModelSkinVerticesFromProjection()
{
}

void *ActionModelSkinVerticesFromProjection::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	foreachi(m->Surface, s, si)
		foreachi(s.Triangle, t, ti)
			for (int k=0;k<3;k++)
				if (m->Vertex[t.Vertex[k]].is_selected){
					surface.add(si);
					triangle.add(ti);
					vert_on_tria.add(k);
					for (int l=0;l<MODEL_MAX_TEXTURES;l++){
						old_pos[l].add(t.SkinVertex[l][k]);
						t.SkinVertex[l][k] = sg.get(m->Vertex[t.Vertex[k]].pos);
					}
				}

	return NULL;
}



void ActionModelSkinVerticesFromProjection::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(surface.num == triangle.num);
	assert(surface.num == vert_on_tria.num);
	assert(surface.num == old_pos[0].num);

	foreachi(surface, s, i)
		for (int l=0;l<MODEL_MAX_TEXTURES;l++)
			m->Surface[s].Triangle[triangle[i]].SkinVertex[l][vert_on_tria[i]] = old_pos[l][i];
}



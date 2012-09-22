/*
 * ActionModelSkinVerticesFromProjection.cpp
 *
 *  Created on: 10.06.2012
 *      Author: michi
 */

#include "ActionModelSkinVerticesFromProjection.h"
#include "../../../../Mode/Model/Mesh/ModeModelMeshTexture.h"
#include <assert.h>

ActionModelSkinVerticesFromProjection::ActionModelSkinVerticesFromProjection(DataModel *m, MultiView *mv)
{
	sg.init_projective(mv, mv->mouse_win);

	// list of selected skin vertices and save old pos
	foreachi(ModelSurface &s, m->Surface, si)
		foreachi(ModelTriangle &t, s.Triangle, ti)
			for (int k=0;k<3;k++)
				if (m->Vertex[t.Vertex[k]].is_selected){
					vert_on_tria.add(k);
					surface.add(si);
					tria.add(ti);
				}
}

ActionModelSkinVerticesFromProjection::~ActionModelSkinVerticesFromProjection()
{
}

void *ActionModelSkinVerticesFromProjection::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	for (int l=0;l<MODEL_MAX_TEXTURES;l++)
		old_pos[l].clear();
	foreachi(int k, vert_on_tria, i){
		ModelTriangle &t = m->Surface[surface[i]].Triangle[tria[i]];
		for (int l=0;l<MODEL_MAX_TEXTURES;l++){
			vector &v = t.SkinVertex[l][k];
			old_pos[l].add(v);
			v = sg.get(m->Vertex[t.Vertex[k]].pos);
		}
	}

	return NULL;
}



void ActionModelSkinVerticesFromProjection::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	foreachi(int k, vert_on_tria, i){
		ModelTriangle &t = m->Surface[surface[i]].Triangle[tria[i]];
		for (int l=0;l<MODEL_MAX_TEXTURES;l++)
			t.SkinVertex[l][k] = old_pos[l][i];
	}
}



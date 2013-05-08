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
		foreachi(ModelPolygon &t, s.Polygon, ti)
			if (t.is_selected)
				for (int k=0;k<t.Side.num;k++){
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

	for (int l=0;l<MATERIAL_MAX_TEXTURES;l++)
		old_pos[l].clear();
	foreachi(int k, vert_on_tria, i){
		ModelPolygon &t = m->Surface[surface[i]].Polygon[tria[i]];
		for (int l=0;l<MATERIAL_MAX_TEXTURES;l++){
			vector &v = t.Side[k].SkinVertex[l];
			old_pos[l].add(v);
			v = sg.get(m->Vertex[t.Side[k].Vertex].pos);
		}
	}

	return NULL;
}



void ActionModelSkinVerticesFromProjection::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	foreachi(int k, vert_on_tria, i){
		ModelPolygon &t = m->Surface[surface[i]].Polygon[tria[i]];
		for (int l=0;l<MATERIAL_MAX_TEXTURES;l++)
			t.Side[k].SkinVertex[l] = old_pos[l][i];
	}
}



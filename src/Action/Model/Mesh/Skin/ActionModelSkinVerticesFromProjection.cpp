/*
 * ActionModelSkinVerticesFromProjection.cpp
 *
 *  Created on: 10.06.2012
 *      Author: michi
 */

#include "ActionModelSkinVerticesFromProjection.h"
#include "../../../../MultiView/MultiView.h"
#include <assert.h>

#include "../../../../Mode/Model/Mesh/ModeModelMeshTexture.h"

ActionModelSkinVerticesFromProjection::ActionModelSkinVerticesFromProjection(DataModel *m, MultiView::MultiView *mv)
{
	sg.init_projective(mv->mouse_win);

	// list of selected skin vertices and save old pos
	foreachi(ModelSurface &s, m->surface, si)
		foreachi(ModelPolygon &t, s.polygon, ti)
			if (t.is_selected)
				for (int k=0;k<t.side.num;k++){
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
		ModelPolygon &t = m->surface[surface[i]].polygon[tria[i]];
		for (int l=0;l<MATERIAL_MAX_TEXTURES;l++){
			vector &v = t.side[k].skin_vertex[l];
			old_pos[l].add(v);
			v = sg.get(m->vertex[t.side[k].vertex].pos);
		}
	}

	return NULL;
}

const string& ActionModelSkinVerticesFromProjection::message()
{
	return DataModel::MESSAGE_SKIN_CHANGE;
}

void ActionModelSkinVerticesFromProjection::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	foreachi(int k, vert_on_tria, i){
		ModelPolygon &t = m->surface[surface[i]].polygon[tria[i]];
		for (int l=0;l<MATERIAL_MAX_TEXTURES;l++)
			t.side[k].skin_vertex[l] = old_pos[l][i];
	}
}



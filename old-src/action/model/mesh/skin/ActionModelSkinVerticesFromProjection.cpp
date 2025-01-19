/*
 * ActionModelSkinVerticesFromProjection.cpp
 *
 *  Created on: 10.06.2012
 *      Author: michi
 */

#include "ActionModelSkinVerticesFromProjection.h"
#include "../../../../lib/math/vec3.h"
#if HAS_LIB_GL
#include "../../../../multiview/MultiView.h"
#endif
#include "../../../../data/model/DataModel.h"
#include "../../../../data/model/ModelMesh.h"
#include "../../../../data/model/ModelPolygon.h"
#include <assert.h>

ActionModelSkinVerticesFromProjection::ActionModelSkinVerticesFromProjection(DataModel *m, MultiView::MultiView *mv) {
#if HAS_LIB_GL
	sg.init_projective(mv->mouse_win);
#endif

	// list of selected skin vertices and save old pos
	foreachi(ModelPolygon &t, m->mesh->polygon, ti)
		if (t.is_selected)
			for (int k=0;k<t.side.num;k++) {
				vert_on_tria.add(k);
				tria.add(ti);
			}
}

void *ActionModelSkinVerticesFromProjection::execute(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	for (int l=0;l<MATERIAL_MAX_TEXTURES;l++)
		old_pos[l].clear();
	foreachi(int k, vert_on_tria, i) {
		ModelPolygon &t = m->mesh->polygon[tria[i]];
		for (int l=0;l<MATERIAL_MAX_TEXTURES;l++) {
			vec3 &v = t.side[k].skin_vertex[l];
			old_pos[l].add(v);
			v = sg.get(m->mesh->vertex[t.side[k].vertex].pos);
		}
	}

	m->out_skin_changed.notify();
	return NULL;
}

void ActionModelSkinVerticesFromProjection::undo(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);

	foreachi(int k, vert_on_tria, i) {
		ModelPolygon &t = m->mesh->polygon[tria[i]];
		for (int l=0;l<MATERIAL_MAX_TEXTURES;l++)
			t.side[k].skin_vertex[l] = old_pos[l][i];
	}
	m->out_skin_changed.notify();
}



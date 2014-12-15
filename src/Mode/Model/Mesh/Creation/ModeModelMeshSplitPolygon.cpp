/*
 * ModeModelMeshSplitPolygon.cpp
 *
 *  Created on: 01.04.2012
 *      Author: michi
 */

#include "ModeModelMeshSplitPolygon.h"
#include "../../../../Edward.h"
#include "../../../../Action/Model/Mesh/Polygon/ActionModelSplitPolygon.h"
#include "../../../../Action/Model/Mesh/Edge/ActionModelSplitEdge.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/Window.h"

ModeModelMeshSplitPolygon::ModeModelMeshSplitPolygon(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshSplitPolygon", _parent)
{
	surface = -1;
	triangle = -1;
	factor = 0;

	message = _("neuen Punkt in Polygon setzen");
}

ModeModelMeshSplitPolygon::~ModeModelMeshSplitPolygon()
{
}

void ModeModelMeshSplitPolygon::onLeftButtonDown()
{
	/*triangle = multi_view->MouseOver;
	surface = multi_view->MouseOverSet;
	pos = multi_view->MouseOverTP;*/

	if ((triangle >= 0) && (surface >= 0)){
		if (edge >= 0)
			data->execute(new ActionModelSplitEdge(surface, edge, factor));
		else
			data->execute(new ActionModelSplitPolygon(surface, triangle, pos));
		//Abort();
		surface = -1;
		triangle = -1;
		ed->forceRedraw();
	}
}

void ModeModelMeshSplitPolygon::onDrawWin(MultiView::Window *win)
{
	triangle = multi_view->hover.index;
	surface = multi_view->hover.set;
	pos = multi_view->hover.point;
	edge = -1;

	if ((triangle >= 0) && (surface >= 0)){
		vector pp = win->project(pos);
		pp.z = 0;
		ModelPolygon &poly = data->surface[surface].polygon[triangle];
		Array<vector> v, p;
		for (int k=0;k<poly.side.num;k++){
			v.add(data->vertex[poly.side[k].vertex].pos);
			p.add(win->project(v[k]));
			p.back().z = 0;
		}

		// close to an edge?
		for (int k=0;k<v.num;k++){
			if (VecLineDistance(pp, p[k], p[(k + 1) % v.num]) < 10){
				float f = (pp - p[k]).length() / (p[k] - p[(k + 1) % v.num]).length();
				pos = v[k] * (1 - f) + v[(k + 1) % v.num] * f;
				pp = win->project(pos);
				edge = poly.side[k].edge;
				factor = f;
			}
		}

		NixEnableLighting(false);
		/*NixDrawLine3D(data->Vertex[data->Surface[surface].Triangle[triangle].Vertex[0]].pos, pos, Green);
		NixDrawLine3D(data->Vertex[data->Surface[surface].Triangle[triangle].Vertex[1]].pos, pos, Green);
		NixDrawLine3D(data->Vertex[data->Surface[surface].Triangle[triangle].Vertex[2]].pos, pos, Green);*/
		NixSetColor(Green);
		for (int k=0;k<v.num;k++)
			NixDrawLine(p[k].x, p[k].y, pp.x, pp.y, 0);
		NixSetColor(White);
		NixEnableLighting(true);
	}
}

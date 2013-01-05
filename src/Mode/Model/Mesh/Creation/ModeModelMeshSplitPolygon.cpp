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

ModeModelMeshSplitPolygon::ModeModelMeshSplitPolygon(Mode *_parent) :
	ModeCreation("ModelMeshSplitPolygon", _parent)
{
	data = (DataModel*)_parent->GetData();

	surface = -1;
	triangle = -1;
	factor = 0;

	message = _("neuen Punkt in Polygon setzen");
}

ModeModelMeshSplitPolygon::~ModeModelMeshSplitPolygon()
{
}

void ModeModelMeshSplitPolygon::OnLeftButtonDown()
{
	/*triangle = multi_view->MouseOver;
	surface = multi_view->MouseOverSet;
	pos = multi_view->MouseOverTP;*/

	if ((triangle >= 0) && (surface >= 0)){
		if (edge >= 0)
			data->Execute(new ActionModelSplitEdge(surface, edge, factor));
		else
			data->Execute(new ActionModelSplitPolygon(surface, triangle, pos));
		//Abort();
		surface = -1;
		triangle = -1;
		ed->ForceRedraw();
	}
}

void ModeModelMeshSplitPolygon::OnDrawWin(int win)
{
	triangle = multi_view->MouseOver;
	surface = multi_view->MouseOverSet;
	pos = multi_view->MouseOverTP;
	edge = -1;

	if ((triangle >= 0) && (surface >= 0)){
		vector pp = multi_view->VecProject(pos, win);
		pp.z = 0;
		ModelPolygon &poly = data->Surface[surface].Polygon[triangle];
		Array<vector> v, p;
		for (int k=0;k<poly.Side.num;k++){
			v.add(data->Vertex[poly.Side[k].Vertex].pos);
			p.add(multi_view->VecProject(v[k], win));
			p.back().z = 0;
		}

		// close to an edge?
		for (int k=0;k<v.num;k++){
			if (VecLineDistance(pp, p[k], p[(k + 1) % v.num]) < 10){
				float f = (pp - p[k]).length() / (p[k] - p[(k + 1) % v.num]).length();
				pos = v[k] * (1 - f) + v[(k + 1) % v.num] * f;
				pp = multi_view->VecProject(pos, win);
				edge = poly.Side[k].Edge;
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

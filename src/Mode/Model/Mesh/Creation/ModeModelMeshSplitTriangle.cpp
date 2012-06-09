/*
 * ModeModelMeshSplitTriangle.cpp
 *
 *  Created on: 01.04.2012
 *      Author: michi
 */

#include "ModeModelMeshSplitTriangle.h"
#include "../../../../Edward.h"
#include "../../../../Action/Model/Mesh/Triangle/ActionModelSplitTriangle.h"
#include "../../../../Action/Model/Mesh/Triangle/ActionModelSplitEdge.h"

ModeModelMeshSplitTriangle::ModeModelMeshSplitTriangle(Mode *_parent, DataModel *_data)
{
	name = "ModelMeshSplitTriangle";
	parent = _parent;
	data = _data;
	multi_view = parent->multi_view;

	surface = -1;
	triangle = -1;

	message = _("neuen Punkt in Dreieck setzen");
}

ModeModelMeshSplitTriangle::~ModeModelMeshSplitTriangle()
{
}

void ModeModelMeshSplitTriangle::OnLeftButtonDown()
{
	/*triangle = multi_view->MouseOver;
	surface = multi_view->MouseOverSet;
	pos = multi_view->MouseOverTP;*/

	if ((triangle >= 0) && (surface >= 0)){
		if (edge >= 0)
			data->Execute(new ActionModelSplitEdge(data, surface, edge, pos));
		else
			data->Execute(new ActionModelSplitTriangle(data, surface, triangle, pos));
		//ed->SetCreationMode(NULL);
		surface = -1;
		triangle = -1;
	}
}

void ModeModelMeshSplitTriangle::PostDrawWin(int win, irect dest)
{
	triangle = multi_view->MouseOver;
	surface = multi_view->MouseOverSet;
	pos = multi_view->MouseOverTP;
	edge = -1;

	if ((triangle >= 0) && (surface >= 0)){
		vector pp = multi_view->VecProject(pos, win);
		pp.z = 0;
		vector v[3], p[3];
		for (int k=0;k<3;k++){
			v[k] = data->Vertex[data->Surface[surface].Triangle[triangle].Vertex[k]].pos;
			p[k] = multi_view->VecProject(v[k], win);
			p[k].z = 0;
		}

		// close to an edge?
		for (int k=0;k<3;k++){
			if (VecLineDistance(pp, p[k], p[(k + 1) % 3]) < 10){
				float f = VecLength(pp - p[k]) / VecLength(p[k] - p[(k + 1) % 3]);
				pos = v[k] * (1 - f) + v[(k + 1) % 3] * f;
				pp = multi_view->VecProject(pos, win);
				edge = data->Surface[surface].Triangle[triangle].Edge[k];
			}
		}

		/*NixDrawLine3D(data->Vertex[data->Surface[surface].Triangle[triangle].Vertex[0]].pos, pos, Green);
		NixDrawLine3D(data->Vertex[data->Surface[surface].Triangle[triangle].Vertex[1]].pos, pos, Green);
		NixDrawLine3D(data->Vertex[data->Surface[surface].Triangle[triangle].Vertex[2]].pos, pos, Green);*/
		for (int k=0;k<3;k++)
			NixDrawLine(p[k].x, p[k].y, pp.x, pp.y, Green, 0);
	}
}

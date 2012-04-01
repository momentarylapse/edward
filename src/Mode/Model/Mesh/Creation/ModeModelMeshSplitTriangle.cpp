/*
 * ModeModelMeshSplitTriangle.cpp
 *
 *  Created on: 01.04.2012
 *      Author: michi
 */

#include "ModeModelMeshSplitTriangle.h"
#include "../../../../Edward.h"

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
}

void ModeModelMeshSplitTriangle::PostDrawWin(int win, irect dest)
{
	triangle = multi_view->MouseOver;
	surface = multi_view->MouseOverSet;
	pos = multi_view->MouseOverTP;

	if ((triangle >= 0) && (surface >= 0)){
		vector pa = multi_view->VecProject(data->Vertex[data->Surface[surface].Triangle[triangle].Vertex[0]].pos, win);
		vector pb = multi_view->VecProject(data->Vertex[data->Surface[surface].Triangle[triangle].Vertex[1]].pos, win);
		vector pc = multi_view->VecProject(data->Vertex[data->Surface[surface].Triangle[triangle].Vertex[2]].pos, win);
		vector pp = multi_view->VecProject(pos, win);

		/*NixDrawLine3D(data->Vertex[data->Surface[surface].Triangle[triangle].Vertex[0]].pos, pos, Green);
		NixDrawLine3D(data->Vertex[data->Surface[surface].Triangle[triangle].Vertex[1]].pos, pos, Green);
		NixDrawLine3D(data->Vertex[data->Surface[surface].Triangle[triangle].Vertex[2]].pos, pos, Green);*/
		NixDrawLine(pa.x, pa.y, pp.x, pp.y, Green, 0);
		NixDrawLine(pb.x, pb.y, pp.x, pp.y, Green, 0);
		NixDrawLine(pc.x, pc.y, pp.x, pp.y, Green, 0);
	}
}

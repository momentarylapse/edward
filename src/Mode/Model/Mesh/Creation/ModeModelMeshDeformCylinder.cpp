/*
 * ModeModelMeshDeformCylinder.cpp
 *
 *  Created on: 26.12.2014
 *      Author: michi
 */

#include "ModeModelMeshDeformCylinder.h"


#include "../../ModeModel.h"
#include "../ModeModelMesh.h"
#include "../../../../Data/Model/Geometry/GeometryCylinder.h"
#include "../../../../Edward.h"
#include "../../../../lib/nix/nix.h"
#include "../../../../lib/script/script.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/Window.h"
#include "../../../../Action/ActionGroup.h"
#include "../../../../Action/Model/Mesh/Vertex/Helper/ActionModelMoveVertex.h"
#include "../Selection/MeshSelectionModePolygon.h"
#include "ModeModelMeshDeformCylinder.h"

const int CYLINDER_EDGES = 24;
const int CYLINDER_RINGS = 24;

class DeformationCylinderDialog : public HuiDialog
{
public:
	DeformationCylinderDialog(ModeModelMeshDeformCylinder *_mode) :
		HuiDialog("", 0, 0, ed, true)
	{
		fromResource("deformation_cylinder_dialog");
		mode = _mode;
	}

	virtual void onDraw()
	{
		HuiPainter *p = beginDraw("area");

		float w = p->width;
		float h = p->height;
		p->setColor(White);
		p->drawRect(0, 0, w, h);
		p->setColor(Black);

		p->drawStr(10, 10, "x");

		p->end();
	}


	ModeModelMeshDeformCylinder *mode;
};

 ModeModelMeshDeformCylinder::ModeModelMeshDeformCylinder(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshDeformCylinder", _parent)
{
	geo = NULL;
	has_preview = false;

	param.add(vector(0, 1, 0));
	param.add(vector(0.5f, 1, 0));
	param.add(vector(1, 1, 0));
}

ModeModelMeshDeformCylinder::~ModeModelMeshDeformCylinder()
{
}

void ModeModelMeshDeformCylinder::onStart()
{
	// Dialog
	dialog = new DeformationCylinderDialog(this);
	dialog->setPositionSpecial(ed, HuiRight | HuiTop);
	dialog->event("hui:close", this, &ModeModelMeshDeformCylinder::onClose);
	dialog->event("ok", this, &ModeModelMeshDeformCylinder::onOk);
	dialog->show();

	//ed->activate("");

	data->getBoundingBox(max, min);
	bool first = true;
	foreachi(ModelVertex &v, data->vertex, i)
		if (v.is_selected){
			index.add(i);
			old_pos.add(v.pos);
			if (first){
				min = v.pos;
				max = v.pos;
			}else{
				min._min(v.pos);
				max._max(v.pos);
			}
			first = false;
		}
	vector d = max - min;
	if ((d.x > d.y) and (d.x > d.z)){
		dir = e_x;
		radius = max(d.y, d.z) / 2;
	}else if (d.y > d.z){
		dir = e_y;
		radius = max(d.x, d.z) / 2;
	}else{
		dir = e_z;
		radius = max(d.x, d.y) / 2;
	}
	vector m = (max + min) / 2;
	axis[0] = m + (dir * (min - m)) * dir;
	axis[1] = m + (dir * (max - m)) * dir;

	geo = new GeometryCylinder(axis[0], axis[1], radius, CYLINDER_RINGS, CYLINDER_EDGES);
	hover = -1;
}

void ModeModelMeshDeformCylinder::onEnd()
{
	if (has_preview)
		restore();
	delete(dialog);
	delete(geo);
}

void ModeModelMeshDeformCylinder::onDrawWin(MultiView::Window* win)
{
	parent->onDrawWin(win);

	mode_model_mesh->setMaterialCreation();
	geo->preview(VBTemp, 1);

	VBTemp->draw();

	NixLineWidth = 3;
	NixSetAlpha(AlphaNone);
	NixSetZ(false, false);
	NixEnableLighting(false);
	NixSetColor(Green);
	NixDrawLine3D(axis[0], axis[1]);

	vector e1 = dir.ortho();
	vector e2 = dir ^ e1;
	foreachi(vector &p, param, ip){
		NixSetColor((ip == hover) ? Red : Green);
		vector m = axis[0] + (axis[1] - axis[0]) * p.x;
		vector v = m + e1 * radius * p.y;
		for (int i=1; i<=CYLINDER_EDGES; i++){
			float ang = (float)i / (float)CYLINDER_EDGES * 2 * pi;
			vector w = m + (e1 * cos(ang) + e2 * sin(ang)) * radius * p.y;
			NixDrawLine3D(w, v);
			v = w;
		}
	}

	NixSetZ(true, true);
	NixEnableLighting(true);
	NixLineWidth = 1;
}

inline bool hover_line(vector &a, vector &b, vector &m)
{
	const float r = 5;
	//if ((a - m).length() < r)
	//	return true;
	if (VecLineDistance(a, b, m) < r){
		vector p = VecLineNearestPoint(m, a, b);
		if (p.between(a, b))
			return true;
	}
	return false;
}

void ModeModelMeshDeformCylinder::updateHover()
{
	hover = -1;
	vector e1 = dir.ortho();
	vector e2 = dir ^ e1;
	foreachi(vector &p, param, ip){
		vector m = axis[0] + (axis[1] - axis[0]) * p.x;
		vector v =  multi_view->mouse_win->project(m + e1 * radius * p.y);
		v.z = 0;
		for (int i=1; i<=CYLINDER_EDGES; i++){
			float ang = (float)i / (float)CYLINDER_EDGES * 2 * pi;
			vector w = multi_view->mouse_win->project(m + (e1 * cos(ang) + e2 * sin(ang)) * radius * p.y);
			w.z = 0;
			if (hover_line(v, w, multi_view->m)){
				hover = ip;
				return;
			}
			v = w;
		}
	}
}

vector ModeModelMeshDeformCylinder::transform(const vector &v)
{
	vector d = max - min;
	vector vv = v - min;
	vv = vector(vv.x / d.x, vv.y / d.y, vv.z / d.z);
	vector w;
	//(*f)(w, vv);
	w = vv;
	return min + vector(w.x * d.x, w.y * d.y, w.z * d.z);
}

void ModeModelMeshDeformCylinder::onPreview()
{
	if (has_preview)
		restore();

	foreach(ModelVertex &v, geo->vertex)
		v.pos = transform(v.pos);

	foreach(int vi, index)
			data->vertex[vi].pos = transform(data->vertex[vi].pos);
	data->notify();
	has_preview = true;
	ed->forceRedraw();
}

void ModeModelMeshDeformCylinder::onMouseMove()
{
	updateHover();
}

void ModeModelMeshDeformCylinder::onLeftButtonDown()
{
}

void ModeModelMeshDeformCylinder::onLeftButtonUp()
{
}

void ModeModelMeshDeformCylinder::restore()
{
	/*delete(geo);
	vector d = max - min;
	geo = new GeometryCube(min, e_x * d.x, e_y * d.y, e_z * d.z, CUBE_SIZE, CUBE_SIZE, CUBE_SIZE);
	ed->forceRedraw();

	foreachi(int vi, index, ii)
		data->vertex[vi].pos = old_pos[ii];
	data->notify();

	has_preview = false;*/
}

void ModeModelMeshDeformCylinder::onOk()
{
	if (has_preview)
		restore();

	data->beginActionGroup("deformation");
	foreach(int vi, index)
		data->execute(new ActionModelMoveVertex(vi, transform(data->vertex[vi].pos)));

	data->endActionGroup();

	abort();
}

void ModeModelMeshDeformCylinder::onClose()
{
	abort();
}

/*
 * ModeModelMeshAutoweld.cpp
 *
 *  Created on: 11.11.2013
 *      Author: michi
 */

#include "ModeModelMeshAutoweld.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/Window.h"

ModeModelMeshAutoweld::ModeModelMeshAutoweld(ModeBase* _parent) :
	ModeCreation<DataModel>("ModelMeshAutoweld", _parent)
{
}

ModeModelMeshAutoweld::~ModeModelMeshAutoweld()
{
}

void ModeModelMeshAutoweld::OnStart()
{
	radius_default = multi_view->cam.radius * 0.01f;
	radius = radius_default;

	dialog = new HuiDialog("Autoweld", 240, 60, ed, true);
	dialog->addControlTable("", 0, 0, 1, 2, "table0");
	dialog->setTarget("table0", 0);
	dialog->addControlTable("", 0, 0, 3, 2, "table1");
	dialog->addControlTable("!buttonbar", 0, 1, 2, 1, "table2");
	dialog->setTarget("table1", 0);
	dialog->addText("Radius", 0, 0, 0, 0, "");
	dialog->addSlider("!expandx", 1, 0, 0, 0, "slider");
	dialog->addText("", 2, 0, 0, 0, "radius");
	dialog->addText("Verbindungen", 0, 1, 0, 0, "");
	dialog->addText("", 1, 1, 0, 0, "num_connections");
	dialog->setTarget("table2", 0);
	dialog->addButton("Abbrechen", 0, 0, 0, 0, "cancel");
	dialog->addButton("Ok", 1, 0, 0, 0, "ok");
	dialog->show();

	dialog->setFloat("slider", 0.5f);
	dialog->setString("radius", f2s(radius, 2));

	dialog->event("slider", this, &ModeModelMeshAutoweld::OnSlider);
	dialog->event("ok", this, &ModeModelMeshAutoweld::OnOk);
	dialog->event("cancel", this, &ModeModelMeshAutoweld::OnCancel);
}

void ModeModelMeshAutoweld::OnEnd()
{
	delete(dialog);
}

void ModeModelMeshAutoweld::OnSlider()
{
	radius = radius_default * exp((dialog->getFloat("slider") - 0.5f) * 5);
	dialog->setString("radius", f2s(radius, 2));
	ed->forceRedraw();
}

void ModeModelMeshAutoweld::OnCancel()
{
	Abort();
}

void ModeModelMeshAutoweld::OnOk()
{
	data->AutoWeldSelectedSurfaces(radius);
	Abort();
}

void ModeModelMeshAutoweld::OnDrawWin(MultiView::Window* win)
{
	NixEnableLighting(false);
	NixSetTexture(NULL);
	NixSetColor(Green);
	float r = 5;
	int n = 0;
	for (int i=0;i<data->Surface.num;i++){
		ModelSurface *a = &data->Surface[i];
		if (!a->is_selected)
			continue;
		for (int j=i+1;j<data->Surface.num;j++){
			ModelSurface *b = &data->Surface[j];
			if (!b->is_selected)
				continue;
			foreach(int va, a->Vertex)
				foreach(int vb, b->Vertex)
					if ((data->Vertex[va].pos - data->Vertex[vb].pos).length() <= radius){
						n ++;
						vector p = win->Project(data->Vertex[va].pos);

						if ((p.z < 0) || (p.z >= 1))
							continue;
						NixDrawRect(	p.x-r,
										p.x+r,
										p.y-r,
										p.y+r,
										p.z);
					}
		}
	}
	dialog->setString("num_connections", i2s(n));
}

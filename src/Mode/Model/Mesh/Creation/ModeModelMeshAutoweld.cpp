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
	radius_default = 1;
	radius = 1;
}

ModeModelMeshAutoweld::~ModeModelMeshAutoweld()
{
}

void ModeModelMeshAutoweld::onStart()
{
	radius_default = multi_view->cam.radius * 0.01f;
	radius = radius_default;

	dialog = new hui::Dialog("Autoweld", 240, 60, ed, true);
	dialog->addGrid("", 0, 0, "table0");
	dialog->setTarget("table0");
	dialog->addGrid("", 0, 0, "table1");
	dialog->addGrid("!buttonbar", 0, 1, "table2");
	dialog->setTarget("table1");
	dialog->addLabel("Radius", 0, 0, "");
	dialog->addSlider("!expandx", 1, 0, "slider");
	dialog->addLabel("", 2, 0, "radius");
	dialog->addLabel("Verbindungen", 0, 1, "");
	dialog->addLabel("", 1, 1, "num_connections");
	dialog->setTarget("table2");
	dialog->addButton("Abbrechen", 0, 0, "cancel");
	dialog->addButton("Ok", 1, 0, "ok");
	dialog->show();

	dialog->setFloat("slider", 0.5f);
	dialog->setString("radius", f2s(radius, 2));

	dialog->event("slider", std::bind(&ModeModelMeshAutoweld::onSlider, this));
	dialog->event("ok", std::bind(&ModeModelMeshAutoweld::onOk, this));
	dialog->event("cancel", std::bind(&ModeModelMeshAutoweld::onCancel, this));
}

void ModeModelMeshAutoweld::onEnd()
{
	delete(dialog);
}

void ModeModelMeshAutoweld::onSlider()
{
	radius = radius_default * exp((dialog->getFloat("slider") - 0.5f) * 5);
	dialog->setString("radius", f2s(radius, 2));
	ed->forceRedraw();
}

void ModeModelMeshAutoweld::onCancel()
{
	abort();
}

void ModeModelMeshAutoweld::onOk()
{
	data->autoWeldSelectedSurfaces(radius);
	abort();
}

void ModeModelMeshAutoweld::onDrawWin(MultiView::Window* win)
{
	parent->onDrawWin(win);

	nix::EnableLighting(false);
	nix::SetTexture(NULL);
	nix::SetColor(Green);
	float r = 5;
	int n = 0;
	for (int i=0;i<data->surface.num;i++){
		ModelSurface *a = &data->surface[i];
		if (!a->is_selected)
			continue;
		for (int j=i+1;j<data->surface.num;j++){
			ModelSurface *b = &data->surface[j];
			if (!b->is_selected)
				continue;
			for (int va: a->vertex)
				for (int vb: b->vertex)
					if ((data->vertex[va].pos - data->vertex[vb].pos).length() <= radius){
						n ++;
						vector p = win->project(data->vertex[va].pos);

						if ((p.z < 0) || (p.z >= 1))
							continue;
						nix::DrawRect(	p.x-r,
										p.x+r,
										p.y-r,
										p.y+r,
										p.z);
					}
		}
	}
	dialog->setString("num_connections", i2s(n));
}

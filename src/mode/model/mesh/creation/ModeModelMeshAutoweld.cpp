/*
 * ModeModelMeshAutoweld.cpp
 *
 *  Created on: 11.11.2013
 *      Author: michi
 */

#include "ModeModelMeshAutoweld.h"
#include "../../../../EdwardWindow.h"
#include "../../../../multiview/MultiView.h"
#include "../../../../multiview/Window.h"
#include "../../../../multiview/ColorScheme.h"
#include "../../../../lib/nix/nix.h"

ModeModelMeshAutoweld::ModeModelMeshAutoweld(ModeModelMesh* _parent) :
	ModeCreation<ModeModelMesh, DataModel>("ModelMeshAutoweld", _parent)
{
	radius_default = 1;
	radius = 1;
}

ModeModelMeshAutoweld::~ModeModelMeshAutoweld()
{
}

void ModeModelMeshAutoweld::on_start()
{
	radius_default = multi_view->cam.radius * 0.01f;
	radius = radius_default;

	dialog = new hui::Dialog("Autoweld", 240, 60, ed, true);
	dialog->add_grid("", 0, 0, "table0");
	dialog->set_target("table0");
	dialog->add_grid("", 0, 0, "table1");
	dialog->add_grid("!buttonbar", 0, 1, "table2");
	dialog->set_target("table1");
	dialog->add_label("Radius", 0, 0, "");
	dialog->add_slider("!expandx", 1, 0, "slider");
	dialog->add_label("", 2, 0, "radius");
	dialog->add_label("Verbindungen", 0, 1, "");
	dialog->add_label("", 1, 1, "num_connections");
	dialog->set_target("table2");
	dialog->add_button("Abbrechen", 0, 0, "cancel");
	dialog->add_button("Ok", 1, 0, "ok");
	dialog->show();

	dialog->set_float("slider", 0.5f);
	dialog->set_string("radius", f2s(radius, 2));

	dialog->event("slider", std::bind(&ModeModelMeshAutoweld::onSlider, this));
	dialog->event("ok", std::bind(&ModeModelMeshAutoweld::onOk, this));
	dialog->event("cancel", std::bind(&ModeModelMeshAutoweld::onCancel, this));
}

void ModeModelMeshAutoweld::on_end()
{
	delete(dialog);
}

void ModeModelMeshAutoweld::onSlider()
{
	radius = radius_default * exp((dialog->get_float("slider") - 0.5f) * 5);
	dialog->set_string("radius", f2s(radius, 2));
	multi_view->force_redraw();
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

void ModeModelMeshAutoweld::on_draw_win(MultiView::Window* win)
{
	parent_untyped->on_draw_win(win);

	nix::bind_texture(0, nullptr);
	float r = 5;
	int n = 0;
#if 0
	set_color(scheme.CREATION_LINE);
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
#endif
	dialog->set_string("num_connections", i2s(n));
}

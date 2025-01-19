/*
 * LightmapDialog.cpp
 *
 *  Created on: 19.05.2013
 *      Author: michi
 */

#include "LightmapDialog.h"
#include "../../../data/world/lightmap/LightmapData.h"
#include "../../../data/world/lightmap/LightmapPhotonMap.h"
#include "../../../data/world/lightmap/LightmapPhotonMapImageSpace.h"
#include "../../../data/world/lightmap/LightmapRayTracing.h"
#include "../../../data/world/lightmap/LightmapRadiosity.h"
#include "../../../data/world/DataWorld.h"
#include "../../../EdwardWindow.h"
#include "../../../storage/Storage.h"
#include <y/world/ModelManager.h>
#include "../../../lib/math/vec2.h"
#include "../../../lib/math/rect.h"

LightmapDialog::LightmapDialog(hui::Window *_parent, bool _allow_parent, DataWorld *_data) :
	hui::Dialog("", 400, 400, _parent, _allow_parent)
{
	from_resource("lightmap_dialog");
	data = _data;

	event("cancel", [=]{ OnClose(); });
	event("hui:close", [=]{ OnClose(); });
	event("ok", [=]{ OnOk(); });
	event("preview", [=]{ OnPreview(); });
	event("resolution", [=]{ OnResolution(); });
	event("lightmap_type", [=]{ OnType(); });
	event("find_new_world", [=]{ OnFindNewWorld(); });

	//LoadData();
	set_float("brightness", 10.0f);
	set_float("exponent", 0.8f);
	set_int("photons", 500000);
	set_int("lightmap_type", 4);
	enable("photons", true);
	//SetString("new_world_name", data->filename.basename().replace(".world", "") + "Lightmap");
	set_string("new_world_name", "temp");

	lmd = new LightmapData(data);

	set_float("resolution", lmd->resolution);
	check("allow_sun", lmd->allow_sun);

	FillList();
	enable("ok", lmd->Models.num > 0);
}

LightmapDialog::~LightmapDialog()
{
	delete(lmd);
}

void LightmapDialog::FillList()
{
	reset("lightmap_list");
	for (LightmapData::Model &m: lmd->Models)
		add_string("lightmap_list", format("%s\\%dx%d\\%f", m.orig_name, m.tex_width, m.tex_height, sqrt(m.area) / m.tex_width));
	for (LightmapData::Terrain &t: lmd->Terrains)
		add_string("lightmap_list", format("%s\\%dx%d\\%f", t.orig_name, t.tex_width, t.tex_height, sqrt(t.area) / t.tex_width));
}

void LightmapDialog::OnClose()
{
	request_destroy();
}

void LightmapDialog::OnType()
{
	enable("photons", (get_int("lightmap_type") == 3));
}

void LightmapDialog::SetData()
{
	lmd->new_world_name = get_string("new_world_name");
	lmd->emissive_brightness = get_float("brightness");
	lmd->color_exponent = get_float("exponent");
	lmd->allow_sun = is_checked("allow_sun");
	lmd->texture_out_dir = Path("Lightmap") | lmd->new_world_name;
	lmd->model_out_dir = Path("Lightmap") | lmd->new_world_name;
}

static Lightmap::Histogram *hist_p;

void OnHistDraw(Painter *c)
{
	//HuiPainter *c = HuiCurWindow->beginDraw("area");
	c->set_font_size(10);
	float w = c->width;
	float h = c->height;
	float hh = h - 40;
	float scale = w / hist_p->max;
	c->set_color(White);
	c->draw_rect(rect(0, w, 0, hh));
	c->set_color(Black);
	c->set_line_width(0.8f);
	c->draw_line({0, hh}, {w, hh});
	float grid_dist_min = 40 / scale; // 40 pixel
	int dec = floor(log10(grid_dist_min)) + 1;
	float d = pow(10.0f, (float)dec);
	if (d > grid_dist_min * 2)
		d /= 2;
	c->set_color(Grey);
	for (float x=0; x<hist_p->max; x+=d){
		c->draw_str({scale * x, hh + 3}, f2s(x, max(0, 1-dec)));
		c->draw_line({scale * x, 0}, {scale * x, hh});
	}
	c->set_color(Black);
	c->set_font("Sans", 12, true, false);
	c->draw_str({w / 2 - 40, hh + 20}, _("Brightness"));
	c->set_line_width(1.5f);
	for (int i=0;i<hist_p->f.num-1;i++)
		c->draw_line({(w * i) / hist_p->f.num, hh - hh * hist_p->f[i]}, {(w * (i + 1)) / hist_p->f.num, hh - hh * hist_p->f[i + 1]});
	//c->end();
}

void OnHistClose()
{
	hui::CurWindow->request_destroy();
}

void ShowHistogram(Lightmap::Histogram &h, hui::Window *root)
{
	hist_p = &h;
	hui::Window *dlg = new hui::Dialog("Histogram", 400, 300, root, false);
	dlg->add_grid("", 0, 0, "table");
	dlg->set_target("table");
	dlg->add_drawing_area("", 0, 0, "area");
	dlg->add_button(_("Close"), 0, 1, "close");
	dlg->set_image("close", "hui:close");
	dlg->event_xp("area", "hui:draw", std::bind(&OnHistDraw, std::placeholders::_1));
	dlg->event("hui:close", &OnHistClose);
	dlg->event("close", &OnHistClose);
	hui::fly(dlg);
}

void LightmapDialog::OnPreview()
{
	SetData();
	Lightmap *lm;
	int type = get_int("lightmap_type");
	if (type == 4){
		lm = new LightmapPhotonMapImageSpace(lmd, get_int("photons"));
	}else if (type == 3){
		lm = new LightmapPhotonMap(lmd, get_int("photons"));
	}else if ((type == 1) or (type == 2)){
		lm = new LightmapRadiosity(lmd);
	}else{
		lm = new LightmapRayTracing(lmd);
	}
	if (lm->Preview()){
		Lightmap::Histogram h = lm->GetHistogram();
		ShowHistogram(h, this);
	}
	delete(lm);
}

void LightmapDialog::OnResolution()
{
	lmd->SetResolution(get_float(""));
	FillList();
}

void LightmapDialog::OnFindNewWorld() {
	/*storage->file_dialog(FD_WORLD, true, true).then([this] (const auto& p) {
		set_string("new_world_name", p.simple.str());
	});*/
}

void LightmapDialog::OnOk()
{
	SetData();
	Lightmap *lm;
	int type = get_int("lightmap_type");
	if (type == 4){
		lm = new LightmapPhotonMapImageSpace(lmd, get_int("photons"));
	}else if (type == 3){
		lm = new LightmapPhotonMap(lmd, get_int("photons"));
	}else if ((type == 1) || (type == 2)){
		lm = new LightmapRadiosity(lmd);
	}else{
		lm = new LightmapRayTracing(lmd);
	}
	bool ok = lm->Create();
	delete(lm);
	if (ok)
		request_destroy();
}


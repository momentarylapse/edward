/*
 * ModeModelMeshBrush.cpp
 *
 *  Created on: 22.08.2013
 *      Author: michi
 */

#include "ModeModelMeshBrush.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../lib/nix/nix.h"
#include "../../../../Action/Model/Mesh/Brush/ActionModelBrushExtrude.h"
#include "../../../../Action/Model/Mesh/Brush/ActionModelBrushSmooth.h"
#include "../../../../Action/Model/Mesh/Brush/ActionModelBrushComplexify.h"



// map from 3d-world-space into UV-space for a polygon
// D: Tp -> Tuv
vector map_uv(DataModel *m, int surf, int poly, const vector &pos, matrix &D) {
	auto &p = m->surface[surf].polygon[poly];

	for (int k=0; k<p.side.num-2; k++) {
		int ta = p.side[k].triangulation[0];
		int tb = p.side[k].triangulation[1];
		int tc = p.side[k].triangulation[2];
		vector a = m->vertex[p.side[ta].vertex].pos;
		vector b = m->vertex[p.side[tb].vertex].pos;
		vector c = m->vertex[p.side[tc].vertex].pos;
		vector sa = p.side[ta].skin_vertex[0];
		vector sb = p.side[tb].skin_vertex[0];
		vector sc = p.side[tc].skin_vertex[0];
		float f, g;
		GetBaryCentric(pos, a, b, c, f, g);
		if (f >= 0 and g >= 0 and f+g <= 1) {
			D = matrix(sb-sa, sc-sa, v_0) * matrix(b-a, c-a, vector::cross(b-a, c-a)).inverse();
			//D = matrix::ID;
			return sa + f * (sb - sa) + g * (sc - sa);
		}
	}
	return v_0;
}


class ActionModelBrushTexturePaint: public Action {
public:
	ActionModelBrushTexturePaint(const vector &_pos, const vector &_n, float _radius, float _depth, int _surf, int _poly, const color &_col) {
		pos = _pos;
		n = _n;
		radius = _radius;
		depth = _depth;
		surf = _surf;
		poly = _poly;
		col = _col;
	}
	string name(){	return "ModelBrushTexturePaint";	}

	void *execute(Data *d) {
		DataModel *m = dynamic_cast<DataModel*>(d);

		auto *tl = m->material[0]->texture_levels[0];

		vector e1 = n.ortho();
		vector e2 = n ^ e1;

		matrix D;
		vector v = map_uv(m, surf, poly, pos, D);
		int xx = clampi(v.x * tl->image->width, 0, tl->image->width);
		int yy = clampf(v.y * tl->image->height, 0, tl->image->height);

		vector f1 = D.transform_normal(e1);
		vector f2 = D.transform_normal(e2);
		float rr = (f1.length_sqr() + f2.length_sqr()) * radius * radius;
		int dx = sqrt(rr) * tl->image->width * 1.1f;
		int dy = sqrt(rr) * tl->image->height * 1.1f;
		int x0 = max(xx - dx, 0);
		int y0 = max(yy - dy, 0);
		int x1 = min(xx + dx, tl->image->width);
		int y1 = min(yy + dy, tl->image->height);

		// just draw a fuzzy circle in UV-space...
		for (int i=x0; i<x1; i++)
			for (int j=y0; j<y1; j++) {
				vector vv = vector((float)i / (float)tl->image->width, (float)j / (float)tl->image->height, 0);
				float dd = (vv - v).length_sqr();
				float a = exp(-pow(dd / rr, 4)*2);
				if (a > 0.001f) {
					color c = tl->image->get_pixel(i, j);
					tl->image->set_pixel(i, j, (1-a) * c + a * col);
				}
			}

		tl->edited = true;
		tl->update_texture();
		m->notify(m->MESSAGE_TEXTURE_CHANGE);

		return NULL;
	}
	void undo(Data *d) {
	}

private:
	vector pos, n;
	float radius, depth;
	int surf, poly;
	color col;
};


ModeModelMeshBrush::ModeModelMeshBrush(ModeBase* _parent) :
	ModeCreation<DataModel>("ModelMeshBrush", _parent)
{
	message = _("auf der Oberfl&ache malen");

	brushing = false;
	distance = 1;
	base_diameter = 1;
	base_depth = 1;
}


Action *ModeModelMeshBrush::getAction()
{
	vector pos = multi_view->hover.point;
	vector n = data->surface[multi_view->hover.set].polygon[multi_view->hover.index].temp_normal;
	float radius = dialog->get_float("diameter") / 2;
	float depth = dialog->get_float("depth");
	int type = dialog->get_int("brush_type");
	auto col = dialog->get_color("color");
	if (ed->get_key(hui::KEY_CONTROL))
		depth = - depth;

	Action *a = NULL;
	if (type == 0)
		a = new ActionModelBrushExtrude(pos, n, radius, depth);
	else if (type == 1)
		a = new ActionModelBrushSmooth(pos, n, radius);
	else if (type == 2)
		a = new ActionModelBrushComplexify(pos, n, radius, depth);
	else if (type == 3)
		a = new ActionModelBrushTexturePaint(pos, n, radius, depth, multi_view->hover.set, multi_view->hover.index, col);
	return a;
}

void ModeModelMeshBrush::apply()
{
	Action *a = getAction();
	data->execute(a);
}

void ModeModelMeshBrush::on_start()
{
	mode_model_mesh->setSelectionMode(mode_model_mesh->selection_mode_polygon);
	mode_model->allowSelectionModes(false);

	multi_view->setAllowAction(false);
	multi_view->setAllowSelect(false);

	// Dialog
	dialog = new hui::Dialog(_("Pinsel"), 400, 240, ed, true);//HuiCreateResourceDialog("new_ball_dialog", ed);
	dialog->add_grid("", 0, 0, "grid");

	dialog->set_target("grid");
	dialog->add_grid("!expandx", 0, 0, "grid1");
	dialog->add_list_view("!nobar\\type", 0, 1, "brush_type");

	dialog->set_target("grid1");
	dialog->add_label(_("Dicke"), 0, 0, "");
	dialog->add_slider("!expandx", 1, 0, "diameter_slider");
	dialog->add_edit("!width=60", 2, 0, "diameter");
	dialog->add_label(_("Tiefe"), 0, 1, "");
	dialog->add_slider("", 1, 1, "depth_slider");
	dialog->add_edit("", 2, 1, "depth");
	dialog->add_label(_("Farbe"), 0, 2, "");
	dialog->add_color_button("", 2,2, "color");

	dialog->event("diameter_slider", std::bind(&ModeModelMeshBrush::onDiameterSlider, this));
	dialog->event("depth_slider", std::bind(&ModeModelMeshBrush::onDepthSlider, this));

	base_diameter = multi_view->cam.radius * 0.2f;
	base_depth = multi_view->cam.radius * 0.02f;

	dialog->add_string("brush_type", _("Ausbeulen/eindellen"));
	dialog->add_string("brush_type", _("Gl&atten"));
	dialog->add_string("brush_type", _("Komplexifizieren"));
	dialog->add_string("brush_type", _("In Textur malen"));
	dialog->set_float("diameter_slider", 0.5f);
	dialog->set_float("depth_slider", 0.5f);
	dialog->set_string("diameter", f2s(base_diameter, 2));
	dialog->set_string("depth", f2s(base_depth, 2));
	dialog->set_int("brush_type", 0);
	dialog->set_position_special(ed, hui::HUI_RIGHT | hui::HUI_TOP);
	dialog->show();
	dialog->event("hui:close", std::bind(&ModeModelMeshBrush::onClose, this));

	ed->activate("");
}

void ModeModelMeshBrush::onDiameterSlider()
{
	float x = dialog->get_float("");
	dialog->set_string("diameter", f2s(base_diameter * exp((x - 0.5f) * 4), 2));
}

void ModeModelMeshBrush::onDepthSlider()
{
	float x = dialog->get_float("");
	dialog->set_string("depth", f2s(base_depth * exp((x - 0.5f) * 2), 2));
}

void ModeModelMeshBrush::on_end()
{
	delete(dialog);
	if (brushing)
		data->end_action_group();
	mode_model->allowSelectionModes(true);
}

void ModeModelMeshBrush::on_left_button_down()
{
	if (multi_view->hover.index < 0)
		return;
	data->begin_action_group("brush");
	vector pos = multi_view->hover.point;
	distance = 0;
	last_pos = pos;
	brushing = true;

	apply();
}

void ModeModelMeshBrush::on_left_button_up()
{
	if (brushing)
		data->end_action_group();
	brushing = false;
}

void ModeModelMeshBrush::on_mouse_move()
{
	if (!brushing)
		return;
	if (multi_view->hover.index < 0)
		return;
	vector pos = multi_view->hover.point;
	float radius = dialog->get_float("diameter") / 2;
	distance += (pos - last_pos).length();
	last_pos = pos;
	if (distance > radius * 0.7f){
		distance = 0;
		apply();
	}
}

void ModeModelMeshBrush::on_draw_win(MultiView::Window* win)
{
	parent->on_draw_win(win);

	if (multi_view->hover.index < 0)
		return;
	vector pos = multi_view->hover.point;
	vector n = data->surface[multi_view->hover.set].polygon[multi_view->hover.index].temp_normal;
	float radius = dialog->get_float("diameter") / 2;

	nix::SetColor(multi_view->ColorCreationLine);
	MultiView::set_wide_lines(2);
	vector e1 = n.ortho();
	vector e2 = n ^ e1;
	e1 *= radius;
	e2 *= radius;
	for (int i=0;i<32;i++){
		float w1 = i * 2 * pi / 32;
		float w2 = (i + 1) * 2 * pi / 32;
		nix::DrawLine3D(pos + sin(w1) * e1 + cos(w1) * e2, pos + sin(w2) * e1 + cos(w2) * e2);
	}
}

void ModeModelMeshBrush::onClose()
{
	abort();
}

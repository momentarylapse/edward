/*
 * ModeFont.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "ModeFont.h"
#include "dialog/FontDialog.h"
#include "../../EdwardWindow.h"
#include "../../Session.h"
#include "../../storage/Storage.h"
#include "../../multiview/MultiView.h"
#include "../../multiview/Window.h"
#include "../../multiview/DrawingHelper.h"
#include "../../lib/nix/nix.h"
#include "../../data/font/DataFont.h"
#include "../../data/font/import/ImporterCairo.h"
#include <y/gui/Font.h>

ModeFont::ModeFont(Session *s, MultiView::MultiView *mv) :
	Mode<ModeFont, DataFont>(s, "Font", nullptr, new DataFont(s), mv, "menu_font"),
	in_data_changed(this, [this] { on_data_update(); })
{
	font = new gui::Font;

	dialog = NULL;
}

ModeFont::~ModeFont() {
}


void ModeFont::on_left_button_down()
{
	vec3 m = multi_view->get_cursor();
	int x=0,y=0;
	foreachi(DataFont::Glyph &g, data->glyph, i){
		int x2 = x + g.Width;
		if (x2 > data->TextureWidth){
			x = 0;
			x2 = g.Width;
			y += data->global.GlyphHeight;
		}

		if ((m.x >= x) and (m.x <= x2) and (m.y >= y) and (m.y <= y + data->global.GlyphHeight)){
			data->Marked = i;
			dialog->LoadData();
			multi_view->force_redraw();
		}
		x = x2;
	}
}



void ModeFont::on_end() {
	data->unsubscribe(this);
	session->win->set_side_panel(nullptr);

	auto *t = session->win->get_toolbar(hui::TOOLBAR_TOP);
	t->reset();
	t->enable(false);
}



void ModeFont::on_draw() {
#if HAS_LIB_GL
	nix::set_z(false, false);
	session->drawing_helper->set_color(White);
	nix::set_shader(session->ctx->default_2d.get());
	session->drawing_helper->draw_rect(0, (float)nix::target_width, nix::target_height * 0.9f, (float)nix::target_height, 0);
	session->drawing_helper->set_color(Black);
//	if (dialog)
//		font->drawStr(0, (float)nix::target_height * 0.9f, 0, (float)nix::target_height * 0.1f, dialog->GetSampleText());
#endif
}



void ModeFont::save_as() {
	session->storage->save_as(data);
}




void ModeFont::save() {
	if (data->filename)
		session->storage->save(data->filename, data);
	else
		save_as();
}



void ModeFont::on_command(const string & id)
{
	if (id == "new")
		_new();
	if (id == "open")
		open();
	if (id == "save")
		save();
	if (id == "save_as")
		save_as();

	if (id == "undo")
		data->undo();
	if (id == "redo")
		data->redo();

	if (id == "import")
		Import();
}



void ModeFont::open() {
	session->storage->open(data).then([this] {
		optimize_view();
	});
}



void ModeFont::_new() {
	session->allow_termination().then([this] {
		data->reset();
		optimize_view();
	});
}



void ModeFont::on_start() {
	session->win->get_toolbar(hui::TOOLBAR_TOP)->set_by_id("font-toolbar");
	auto t = session->win->get_toolbar(hui::TOOLBAR_LEFT);
	t->reset();
	t->enable(false);

	multi_view->set_allow_select(false);

	dialog = new FontDialog(data);
	session->win->set_side_panel(dialog);


	data->out_changed >> in_data_changed;
	on_data_update();
}



void ModeFont::on_mouse_move()
{
}



void ModeFont::on_data_update() {
	data->UpdateTexture();
	data->ApplyFont(font);
}



// FIXME argh.... should be unnecessary thanks to better projection system now!!!!

void Draw2D(MultiView::Window *win, const rect &source, const rect *dest) {
#if HAS_LIB_GL
	MultiView::MultiView *mv = win->multi_view;
	rect d;
	if (dest) {
		d=rect(	nix::target_width/2-(mv->cam.pos.x-dest->x1)*mv->active_win->zoom(),
				nix::target_width/2-(mv->cam.pos.x-dest->x2)*mv->active_win->zoom(),
				nix::target_height/2-(mv->cam.pos.y-dest->y1)*mv->active_win->zoom(),
				nix::target_height/2-(mv->cam.pos.y-dest->y2)*mv->active_win->zoom());
		win->drawing_helper->draw_2d(source, d, 0);
	} else {
		win->drawing_helper->draw_2d(source, nix::target_rect, 0);
	}
#endif
}

void DrawLineH(MultiView::Window *win, float x1, float x2, float y) {
#if HAS_LIB_GL
	MultiView::MultiView *mv = win->multi_view;
	x1 = nix::target_width/2-(mv->cam.pos.x - x1)*mv->active_win->zoom();
	x2 = nix::target_width/2-(mv->cam.pos.x - x2)*mv->active_win->zoom();
	y  = nix::target_height/2-(mv->cam.pos.y - y )*mv->active_win->zoom();
	win->drawing_helper->draw_line_2d(x1, y, x2, y, 0);
#endif
}

void DrawLineV(MultiView::Window *win, float x, float y1, float y2) {
#if HAS_LIB_GL
	MultiView::MultiView *mv = win->multi_view;
	x  = nix::target_width/2-(mv->cam.pos.x - x )*mv->active_win->zoom();
	y1 = nix::target_height/2-(mv->cam.pos.y - y1)*mv->active_win->zoom();
	y2 = nix::target_height/2-(mv->cam.pos.y - y2)*mv->active_win->zoom();
	win->drawing_helper->draw_line_2d(x, y1, x, y2, 0);
#endif
}


void ModeFont::on_draw_win(MultiView::Window *win)
{
#if HAS_LIB_GL
	int NumY = data->TextureHeight / data->global.GlyphHeight;

	// background
	nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
	nix::set_z(false, false);
	rect d = rect(0, (float)data->TextureWidth, 0, (float)data->TextureHeight);
	nix::bind_texture(0, nullptr);
	win->drawing_helper->set_color(White);
	Draw2D(win, rect::ID, &d);
	nix::bind_texture(0, data->texture.get());
	win->drawing_helper->set_color(Black);
	Draw2D(win, rect::ID, &d);
	nix::bind_texture(0, nullptr);

	// grid (horizontal lines)
	win->drawing_helper->set_line_width(1.0f);
	for (int i=0;i<NumY;i++){
		win->drawing_helper->set_color(color(0.3f,0.0f,0.8f,0.0f));
		DrawLineH(win, 0, data->TextureWidth,i*data->global.GlyphHeight+data->global.GlyphY1);
		win->drawing_helper->set_color(color(0.3f,0.0f,1.0f,0.0f));
		DrawLineH(win, 0, data->TextureWidth,i*data->global.GlyphHeight+data->global.GlyphY2);
		win->drawing_helper->set_color(color(0.5f,0.5f,0.5f,0.5f));
		DrawLineH(win, 0, data->TextureWidth,(i+1)*data->global.GlyphHeight);
	}

	// separate glyphs
	float x=0,y=0;
	foreachi(DataFont::Glyph &g, data->glyph, i){
		float x2 = x + g.Width;
		if (x2 > data->TextureWidth){
			x = 0;
			x2 = g.Width;
			y += data->global.GlyphHeight;
		}
		if (i == data->Marked){
			d = rect(x, x2, y, y + data->global.GlyphHeight);
			win->drawing_helper->set_color(color(0.2f,1,0,0));
			Draw2D(win, rect::ID, &d);
		}
		win->drawing_helper->set_line_width(1.0f);
		win->drawing_helper->set_color(color(0.3f,0.8f,0.0f,0.0f));
		DrawLineV(win, x + g.X1, y + data->global.GlyphY1, y + data->global.GlyphY2);
		win->drawing_helper->set_color(color(0.3f,0.8f,0.0f,0.0f));
		DrawLineV(win, x + g.X2, y + data->global.GlyphY1, y + data->global.GlyphY2);
		win->drawing_helper->set_color(color(0.3f,0.5f,0.5f,0.5f));
		DrawLineV(win, x2, y, y + data->global.GlyphHeight);
		x = x2;
	}

	nix::disable_alpha();
#endif
}


void ModeFont::on_update_menu()
{
}

bool ModeFont::optimize_view()
{
	multi_view->reset();
	multi_view->set_view_box(v_0, vec3(data->TextureWidth, data->TextureHeight, 0));
	return true;
}

void ModeFont::Import() {
	hui::select_font(session->win, _("Import font"), {}).then([this] (const string &font) {
		ImporterCairo imp;
		imp.Import(data, font);
	});
}


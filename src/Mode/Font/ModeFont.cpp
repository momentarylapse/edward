/*
 * ModeFont.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "../../Edward.h"
#include "../../MultiView/MultiView.h"
#include "../../MultiView/Window.h"
#include "ModeFont.h"
#include "../../Data/Font/DataFont.h"
#include "../../Data/Font/Import/ImporterCairo.h"
#include "Dialog/FontDialog.h"
#include "../../x/font.h"

ModeFont *mode_font = NULL;

ModeFont::ModeFont() :
	Mode("Font", NULL, new DataFont, ed->multi_view_2d, "menu_font")
{
	subscribe(data);

	font = new Gui::Font;

	dialog = NULL;
}

ModeFont::~ModeFont()
{
	unsubscribe(data);
}


void ModeFont::onLeftButtonDown()
{
	vector m = multi_view->getCursor3d();
	int x=0,y=0;
	foreachi(DataFont::Glyph &g, data->glyph, i){
		int x2 = x + g.Width;
		if (x2 > data->TextureWidth){
			x = 0;
			x2 = g.Width;
			y += data->global.GlyphHeight;
		}

		if ((m.x >= x) && (m.x <= x2) && (m.y >= y) && (m.y <= y + data->global.GlyphHeight)){
			data->Marked = i;
			dialog->LoadData();
			ed->forceRedraw();
		}
		x = x2;
	}
}



void ModeFont::onEnd()
{
	delete(dialog);

	hui::Toolbar *t = ed->toolbar[hui::TOOLBAR_TOP];
	t->reset();
	t->enable(false);
}



void ModeFont::onDraw()
{
	nix::SetZ(false, false);
	nix::EnableLighting(false);
	nix::SetColor(White);
	nix::DrawRect(0, (float)nix::target_width, nix::target_height * 0.9f, (float)nix::target_height, 0);
	nix::SetColor(Black);
	if (dialog)
		font->drawStr(0, (float)nix::target_height * 0.9f, 0, (float)nix::target_height * 0.1f, dialog->GetSampleText());
}



bool ModeFont::saveAs()
{
	if (ed->fileDialog(FD_FONT, true, false))
		return data->save(ed->dialog_file_complete);
	return false;
}




bool ModeFont::save()
{
	if (data->filename == "")
		return saveAs();
	return data->save(data->filename);
}



void ModeFont::onCommand(const string & id)
{
	if (id == "new")
		_new();
	if (id == "open")
		open();
	if (id == "save")
		save();
	if (id == "save_as")
		saveAs();

	if (id == "undo")
		data->undo();
	if (id == "redo")
		data->redo();

	if (id == "import")
		Import();
}



bool ModeFont::open()
{
	if (!ed->allowTermination())
		return false;
	if (!ed->fileDialog(FD_FONT, false, false))
		return false;
	bool ok = data->load(ed->dialog_file_complete);
	if (!ok)
		return false;

	optimizeView();
	ed->setMode(mode_font);
	return true;
}



void ModeFont::_new()
{
	if (!ed->allowTermination())
		return;

	data->reset();
	optimizeView();
	ed->setMode(mode_font);
}



void ModeFont::onStart()
{
	string dir = (app->directory_static + "Data/icons/toolbar/").sys_filename();
	hui::Toolbar *t = ed->toolbar[hui::TOOLBAR_TOP];
	t->reset();
	t->addItem(L("", "new"),dir + "new.png","new");
	t->addItem(L("", "open"),dir + "open.png","open");
	t->addItem(L("", "save"),dir + "save.png","save");
	t->addSeparator();
	t->addItem(L("", "undo"),dir + "undo.png","undo");
	t->addItem(L("", "redo"),dir + "redo.png","redo");
	t->enable(true);
	t->configure(false,true);
	t = ed->toolbar[hui::TOOLBAR_LEFT];
	t->reset();
	t->enable(false);

	dialog = new FontDialog(ed, data);

	onUpdate(data, "");
}



void ModeFont::onMouseMove()
{
}



void ModeFont::onUpdate(Observable *o, const string &message)
{
	data->UpdateTexture();
	data->ApplyFont(font);
}



// FIXME argh.... should be unnecessary thanks to better projection system now!!!!

void Draw2D(const rect &source, const rect *dest)
{
	MultiView::MultiView *mv = mode_font->multi_view;
	rect d;
	if (dest){
		d=rect(	nix::target_width/2-(mv->cam.pos.x-dest->x1)*mv->active_win->zoom(),
				nix::target_width/2-(mv->cam.pos.x-dest->x2)*mv->active_win->zoom(),
				nix::target_height/2-(mv->cam.pos.y-dest->y1)*mv->active_win->zoom(),
				nix::target_height/2-(mv->cam.pos.y-dest->y2)*mv->active_win->zoom());
		nix::Draw2D(source, d, 0);
	}else
		nix::Draw2D(source, nix::target_rect, 0);
}

void DrawLineH(int x1, int x2, int y)
{
	MultiView::MultiView *mv = mode_font->multi_view;
	x1 = int(nix::target_width/2-(mv->cam.pos.x - x1)*mv->active_win->zoom());
	x2 = int(nix::target_width/2-(mv->cam.pos.x - x2)*mv->active_win->zoom());
	y  = int(nix::target_height/2-(mv->cam.pos.y - y )*mv->active_win->zoom());
	nix::DrawLineH(x1, x2,y, 0);
}

void DrawLineV(int x, int y1, int y2)
{
	MultiView::MultiView *mv = mode_font->multi_view;
	x  = int(nix::target_width/2-(mv->cam.pos.x - x )*mv->active_win->zoom());
	y1 = int(nix::target_height/2-(mv->cam.pos.y - y1)*mv->active_win->zoom());
	y2 = int(nix::target_height/2-(mv->cam.pos.y - y2)*mv->active_win->zoom());
	nix::DrawLineV(x, y1, y2, 0);
}


void ModeFont::onDrawWin(MultiView::Window *win)
{
	int NumY = data->TextureHeight / data->global.GlyphHeight;

	// background
	nix::EnableLighting(false);
	nix::SetAlpha(AlphaMaterial);
	nix::SetZ(false, false);
	rect d = rect(0, (float)data->TextureWidth, 0, (float)data->TextureHeight);
	nix::SetTexture(NULL);
	nix::SetColor(White);
	Draw2D(r_id, &d);
	nix::SetTexture(data->Texture);
	nix::SetColor(Black);
	Draw2D(r_id, &d);
	nix::SetTexture(NULL);

	// grid (horizontal lines)
	for (int i=0;i<NumY;i++){
		nix::SetColor(color(0.3f,0.0f,0.8f,0.0f));
		DrawLineH(0, data->TextureWidth,i*data->global.GlyphHeight+data->global.GlyphY1);
		nix::SetColor(color(0.3f,0.0f,1.0f,0.0f));
		DrawLineH(0, data->TextureWidth,i*data->global.GlyphHeight+data->global.GlyphY2);
		nix::SetColor(color(0.5f,0.5f,0.5f,0.5f));
		DrawLineH(0, data->TextureWidth,(i+1)*data->global.GlyphHeight);
	}

	// separate glyphs
	int x=0,y=0;
	foreachi(DataFont::Glyph &g, data->glyph, i){
		int x2 = x + g.Width;
		if (x2 > data->TextureWidth){
			x = 0;
			x2 = g.Width;
			y += data->global.GlyphHeight;
		}
		if (i == data->Marked){
			d = rect(float(x), float(x2), float(y), float(y + data->global.GlyphHeight));
			nix::SetColor(color(0.2f,1,0,0));
			Draw2D(r_id, &d);
		}
		nix::SetColor(color(0.3f,0.8f,0.0f,0.0f));
		DrawLineV(x + g.X1, y + data->global.GlyphY1, y + data->global.GlyphY2);
		nix::SetColor(color(0.3f,0.8f,0.0f,0.0f));
		DrawLineV(x + g.X2, y + data->global.GlyphY1, y + data->global.GlyphY2);
		nix::SetColor(color(0.3f,0.5f,0.5f,0.5f));
		DrawLineV(x2, y, y + data->global.GlyphHeight);
		x = x2;
	}

	nix::SetAlpha(AlphaNone);
}


void ModeFont::onUpdateMenu()
{
}

bool ModeFont::optimizeView()
{
	multi_view->reset();
	multi_view->setViewBox(v_0, vector(data->TextureWidth, data->TextureHeight, 0));
	return true;
}

void ModeFont::Import()
{
	if (hui::SelectFont(ed, _("Font importieren"))){
		ImporterCairo imp;
		imp.Import(data, hui::Fontname);
	}
}


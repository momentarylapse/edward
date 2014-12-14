/*
 * ModeFont.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "../../Edward.h"
#include "../../MultiView/MultiView.h"
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
	vector m = multi_view->GetCursor3d();
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

	HuiToolbar *t = ed->toolbar[HuiToolbarTop];
	t->reset();
	t->enable(false);
}



void ModeFont::onDraw()
{
	NixSetZ(false, false);
	NixEnableLighting(false);
	NixSetColor(White);
	NixDrawRect(0, (float)MaxX, MaxY * 0.9f, (float)MaxY, 0);
	NixSetColor(Black);
	if (dialog)
		font->drawStr(0, (float)MaxY * 0.9f, 0, (float)MaxY * 0.1f, dialog->GetSampleText());
}



bool ModeFont::saveAs()
{
	if (ed->fileDialog(FDFont, true, false))
		return data->save(ed->DialogFileComplete);
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
	if (!ed->fileDialog(FDFont, false, false))
		return false;
	bool ok = data->load(ed->DialogFileComplete);
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
	string dir = (HuiAppDirectoryStatic + "Data/icons/toolbar/").sys_filename();
	HuiToolbar *t = ed->toolbar[HuiToolbarTop];
	t->reset();
	t->addItem(L("new"),dir + "new.png","new");
	t->addItem(L("open"),dir + "open.png","open");
	t->addItem(L("save"),dir + "save.png","save");
	t->addSeparator();
	t->addItem(L("undo"),dir + "undo.png","undo");
	t->addItem(L("redo"),dir + "redo.png","redo");
	t->enable(true);
	t->configure(false,true);
	t = ed->toolbar[HuiToolbarLeft];
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




void Draw2D(const rect &source, const rect *dest)
{
	MultiView::MultiView *mv = mode_font->multi_view;
	rect d;
	if (dest){
		d=rect(	MaxX/2-(mv->cam.pos.x-dest->x1)*mv->cam.zoom,
				MaxX/2-(mv->cam.pos.x-dest->x2)*mv->cam.zoom,
				MaxY/2-(mv->cam.pos.y-dest->y1)*mv->cam.zoom,
				MaxY/2-(mv->cam.pos.y-dest->y2)*mv->cam.zoom);
		NixDraw2D(source, d, 0);
	}else
		NixDraw2D(source, NixTargetRect, 0);
}

void DrawLineH(int x1, int x2, int y)
{
	MultiView::MultiView *mv = mode_font->multi_view;
	x1 = int(MaxX/2-(mv->cam.pos.x - x1)*mv->cam.zoom);
	x2 = int(MaxX/2-(mv->cam.pos.x - x2)*mv->cam.zoom);
	y  = int(MaxY/2-(mv->cam.pos.y - y )*mv->cam.zoom);
	NixDrawLineH(x1, x2,y, 0);
}

void DrawLineV(int x, int y1, int y2)
{
	MultiView::MultiView *mv = mode_font->multi_view;
	x  = int(MaxX/2-(mv->cam.pos.x - x )*mv->cam.zoom);
	y1 = int(MaxY/2-(mv->cam.pos.y - y1)*mv->cam.zoom);
	y2 = int(MaxY/2-(mv->cam.pos.y - y2)*mv->cam.zoom);
	NixDrawLineV(x, y1, y2, 0);
}


void ModeFont::onDrawWin(MultiView::Window *win)
{
	int NumY = data->TextureHeight / data->global.GlyphHeight;

	// background
	NixEnableLighting(false);
	NixSetAlpha(AlphaMaterial);
	NixSetZ(false, false);
	rect d = rect(0, (float)data->TextureWidth, 0, (float)data->TextureHeight);
	NixSetTexture(NULL);
	NixSetColor(White);
	Draw2D(r_id, &d);
	NixSetTexture(data->Texture);
	NixSetColor(Black);
	Draw2D(r_id, &d);
	NixSetTexture(NULL);

	// grid (horizontal lines)
	for (int i=0;i<NumY;i++){
		NixSetColor(color(0.3f,0.0f,0.8f,0.0f));
		DrawLineH(0, data->TextureWidth,i*data->global.GlyphHeight+data->global.GlyphY1);
		NixSetColor(color(0.3f,0.0f,1.0f,0.0f));
		DrawLineH(0, data->TextureWidth,i*data->global.GlyphHeight+data->global.GlyphY2);
		NixSetColor(color(0.5f,0.5f,0.5f,0.5f));
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
			NixSetColor(color(0.2f,1,0,0));
			Draw2D(r_id, &d);
		}
		NixSetColor(color(0.3f,0.8f,0.0f,0.0f));
		DrawLineV(x + g.X1, y + data->global.GlyphY1, y + data->global.GlyphY2);
		NixSetColor(color(0.3f,0.8f,0.0f,0.0f));
		DrawLineV(x + g.X2, y + data->global.GlyphY1, y + data->global.GlyphY2);
		NixSetColor(color(0.3f,0.5f,0.5f,0.5f));
		DrawLineV(x2, y, y + data->global.GlyphHeight);
		x = x2;
	}

	NixSetAlpha(AlphaNone);
}


void ModeFont::onUpdateMenu()
{
}

bool ModeFont::optimizeView()
{
	multi_view->Reset();
	multi_view->SetViewBox(v_0, vector(data->TextureWidth, data->TextureHeight, 0));
	return true;
}

void ModeFont::Import()
{
	if (HuiSelectFont(ed, _("Font importieren"))){
		ImporterCairo imp;
		imp.Import(data, HuiFontname);
	}
}


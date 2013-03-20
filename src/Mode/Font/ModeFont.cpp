/*
 * ModeFont.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "../../Edward.h"
#include "ModeFont.h"
#include "../../Data/Font/DataFont.h"
#include "../../lib/x/x.h"

ModeFont *mode_font = NULL;

ModeFont::ModeFont() :
	Mode("Font", NULL, new DataFont, ed->multi_view_2d, "menu_font")
{
	data = dynamic_cast<DataFont*>(data_generic);
	Subscribe(data);

	SampleText = _("Beispiel Text 0123456789");
	XFont *f = new XFont;
	_XFont_.add(f);
}

ModeFont::~ModeFont()
{
	Unsubscribe(data);
}

void ModeFont::OnKeyDown()
{
}



void ModeFont::OnLeftButtonDown()
{
}



void ModeFont::OnEnd()
{
	ed->ToolbarSetCurrent(HuiToolbarTop);
	ed->ToolbarReset();
	ed->EnableToolbar(false);
}



void ModeFont::OnDraw()
{
	NixSetZ(false, false);
	NixEnableLighting(false);
	NixSetColor(White);
	NixDrawRect(0, (float)MaxX, MaxY * 0.9f, (float)MaxY, 0);
	XFontIndex = _XFont_.num - 1;
	NixSetColor(Black);
	XFDrawStr(0, (float)MaxY * 0.9f, (float)MaxY * 0.1f, SampleText);
}



bool ModeFont::SaveAs()
{
	if (ed->FileDialog(FDFont, true, false))
		return data->Save(ed->DialogFileComplete);
	return false;
}



void ModeFont::OnKeyUp()
{
}



void ModeFont::OnMiddleButtonUp()
{
}



bool ModeFont::Save()
{
	if (data->filename == "")
		return SaveAs();
	return data->Save(data->filename);
}



void ModeFont::OnCommand(const string & id)
{
	if (id == "new")
		New();
	if (id == "open")
		Open();
	if (id == "save")
		Save();
	if (id == "save_as")
		SaveAs();

	if (id == "undo")
		data->Undo();
	if (id == "redo")
		data->Redo();
}



bool ModeFont::Open()
{
	if (!ed->AllowTermination())
		return false;
	if (!ed->FileDialog(FDFont, false, false))
		return false;
	bool ok = data->Load(ed->DialogFileComplete);
	if (!ok)
		return false;

	multi_view->ResetView();
	ed->SetMode(mode_font);
	return true;
}



void ModeFont::OnRightButtonUp()
{
}



void ModeFont::New()
{
	if (!ed->AllowTermination())
		return;

	data->Reset();
	multi_view->ResetView();
	ed->SetMode(mode_font);
}



void ModeFont::OnStart()
{
	string dir = (HuiAppDirectoryStatic + "Data/icons/toolbar/").sys_filename();
	ed->ToolbarSetCurrent(HuiToolbarTop);
	ed->ToolbarReset();
	ed->ToolbarAddItem(L("new"),L("new"),dir + "new.png","new");
	ed->ToolbarAddItem(L("open"),L("open"),dir + "open.png","open");
	ed->ToolbarAddItem(L("save"),L("save"),dir + "save.png","save");
	ed->ToolbarAddSeparator();
	ed->ToolbarAddItem(L("undo"),L("undo"),dir + "undo.png","undo");
	ed->ToolbarAddItem(L("redo"),L("redo"),dir + "redo.png","redo");
	ed->EnableToolbar(true);
	ed->ToolbarConfigure(false,true);
	ed->ToolbarSetCurrent(HuiToolbarLeft);
	ed->ToolbarReset();
	ed->EnableToolbar(false);

	OnUpdate(data);
}



void ModeFont::OnMouseMove()
{
}



void ModeFont::OnUpdate(Observable *o)
{
	data->ApplyFont(_XFont_.back());
}



void ModeFont::OnLeftButtonUp()
{
}



void ModeFont::OnRightButtonDown()
{
}



void Draw2D(const rect &source, const rect *dest)
{
	MultiView *mv = mode_font->multi_view;
	rect d;
	if (dest){
		d=rect(	MaxX/2-(mv->pos.x-dest->x1)*mv->zoom,
				MaxX/2-(mv->pos.x-dest->x2)*mv->zoom,
				MaxY/2-(mv->pos.y-dest->y1)*mv->zoom,
				MaxY/2-(mv->pos.y-dest->y2)*mv->zoom);
		NixDraw2D(source, d, 0);
	}else
		NixDraw2D(source, NixTargetRect, 0);
}

void DrawLineH(int x1, int x2, int y)
{
	MultiView *mv = mode_font->multi_view;
	x1 = int(MaxX/2-(mv->pos.x - x1)*mv->zoom);
	x2 = int(MaxX/2-(mv->pos.x - x2)*mv->zoom);
	y  = int(MaxY/2-(mv->pos.y - y )*mv->zoom);
	NixDrawLineH(x1, x2,y, 0);
}

void DrawLineV(int x, int y1, int y2)
{
	MultiView *mv = mode_font->multi_view;
	x  = int(MaxX/2-(mv->pos.x - x )*mv->zoom);
	y1 = int(MaxY/2-(mv->pos.y - y1)*mv->zoom);
	y2 = int(MaxY/2-(mv->pos.y - y2)*mv->zoom);
	NixDrawLineV(x, y1, y2, 0);
}


void ModeFont::OnDrawWin(int win)
{
	int NumY = data->TextureHeight / data->GlyphHeight;

	// background
	NixEnableLighting(false);
	NixSetAlpha(AlphaMaterial);
	NixSetZ(false, false);
	rect d = rect(0, (float)data->TextureWidth, 0, (float)data->TextureHeight);
	NixSetTexture(-1);
	NixSetColor(White);
	Draw2D(r_id, &d);
	NixSetTexture(data->Texture);
	NixSetColor(Black);
	Draw2D(r_id, &d);
	NixSetTexture(-1);

	// grid (horizontal lines)
	for (int i=0;i<NumY;i++){
		NixSetColor(color(0.3f,0.0f,0.8f,0.0f));
		DrawLineH(0, data->TextureWidth,i*data->GlyphHeight+data->GlyphY1);
		NixSetColor(color(0.3f,0.0f,1.0f,0.0f));
		DrawLineH(0, data->TextureWidth,i*data->GlyphHeight+data->GlyphY2);
		NixSetColor(color(0.5f,0.5f,0.5f,0.5f));
		DrawLineH(0, data->TextureWidth,(i+1)*data->GlyphHeight);
	}

	// separate glyphs
	int x=0,y=0;
	foreachi(FontGlyph &g, data->Glyph, i){
		int x2 = x + g.Width;
		if (x2 > data->TextureWidth){
			x = 0;
			x2 = g.Width;
			y += data->GlyphHeight;
		}
		if (i == data->Marked){
			d = rect(float(x), float(x2), float(y), float(y + data->GlyphHeight));
			NixSetColor(color(0.2f,1,0,0));
			Draw2D(r_id, &d);
		}
		NixSetColor(color(0.3f,0.8f,0.0f,0.0f));
		DrawLineV(x + g.X1, y + data->GlyphY1, y + data->GlyphY2);
		NixSetColor(color(0.3f,0.8f,0.0f,0.0f));
		DrawLineV(x + g.X2, y + data->GlyphY1, y + data->GlyphY2);
		NixSetColor(color(0.3f,0.5f,0.5f,0.5f));
		DrawLineV(x2, y, y + data->GlyphHeight);
		x = x2;
	}

	NixSetAlpha(AlphaNone);
}



void ModeFont::OnMiddleButtonDown()
{
}



void ModeFont::OnUpdateMenu()
{
}


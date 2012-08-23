/*
 * ModeWelcome.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "../../Edward.h"
#include "ModeWelcome.h"
#include "../Administration/ModeAdministration.h"
#include "../Model/ModeModel.h"
#include "../Material/ModeMaterial.h"
#include "../World/ModeWorld.h"
#include "../Font/ModeFont.h"
#include "../../lib/nix/nix.h"

ModeWelcome *mode_welcome = NULL;

ModeWelcome::ModeWelcome():
	Mode("Welcome", NULL, NULL, NULL, "")
{
	tex_icons = NixLoadTexture(HuiAppDirectoryStatic + "Data/icons/icons.tga");
	tex_logo = NixLoadTexture(HuiAppDirectoryStatic + "Data/edward.jpg");
}

ModeWelcome::~ModeWelcome()
{
}

void ModeWelcome::OnStart()
{
	ed->ToolbarSetCurrent(HuiToolbarTop);
	ed->ToolbarReset();
	ed->EnableToolbar(false);
	ed->ToolbarSetCurrent(HuiToolbarLeft);
	ed->ToolbarReset();
	ed->EnableToolbar(false);

	mouse_over = mouse_over_cmd = -1;
}

void ModeWelcome::OnEnd()
{
}

void ModeWelcome::OnLeftButtonUp()
{
	if (mouse_over == 0){
		if (mouse_over_cmd == 0)
			mode_model->New();
		else
			mode_model->Open();
	}else if (mouse_over == 1){
		if (mouse_over_cmd == 0)
			mode_material->New();
		else
			mode_material->Open();
	}else if (mouse_over == 2){
		if (mouse_over_cmd == 0)
			mode_world->New();
		else
			mode_world->Open();
	}else if (mouse_over == 3){
		if (mouse_over_cmd == 0)
			mode_font->New();
		else
			mode_font->Open();
	}else if (mouse_over == 4){
		ed->SetMode(mode_administration);
	}
}

void ModeWelcome::OnCommand(const string & id)
{
	if (id == "new")
		mode_model->New();
	if (id == "open")
		mode_model->Open();
}

bool mouse_in_rect(irect r, int mx, int my)
{
	return ((mx > r.x1) && (mx < r.x2) && (my > r.y1) && (my < r.y2));
}

void ModeWelcome::OnMouseMove()
{
	int mx = HuiGetEvent()->mx;
	int my = HuiGetEvent()->my;
	mouse_over = -1;
	mouse_over_cmd = -1;
	for (int i=0;i<5;i++)
		if (mouse_in_rect(icon_rect[i], mx, my)){
			mouse_over = i;
			mouse_over_cmd = (my < (icon_rect[i].y1 + icon_rect[i].y2) / 2) ? 0 : 1;
		}

	ed->ForceRedraw();
}


void ModeWelcome::OnUpdate(Observable *o)
{
}

void ModeWelcome::DrawIcon(int x, int y, int index, int pic_index, const string &name)
{
	msg_db_r("DrawIcon", 2);
	rect s,d;
	y+=30;
	float rad=25;
	icon_rect[index] = irect(x-40,x+60,y-30,y+30);
	s.x1=(float)pic_index*0.125f;
	s.x2=(float)pic_index*0.125f+0.125f;
	s.y1=0;
	s.y2=1;

	// the icon itself
	d.x1=(float)x-rad;
	d.x2=(float)x+rad;
	d.y1=(float)y-rad;
	d.y2=(float)y+rad;
	NixSetAlpha(AlphaMaterial);
	NixSetTexture(tex_icons);
	NixSetColor(color((mouse_over==index)?0.3f:0.99f,1,1,1));
	NixDraw2D(s, d, 0);
	NixSetTexture(-1);
	NixSetAlpha(AlphaNone);

	x-=15;
	NixSetColor(color(1,0.6f,0.6f,0.6f));
	if (mouse_over == index){
		// operations to choose from
		if (index != 4){
			color c=color(1,0.5f,0.5f,0.6f); // unmarked: grayish
			NixSetColor((mouse_over_cmd == 0) ? White : c);
			NixDrawStr(x+17,y-25,L("new"));
			NixSetColor((mouse_over_cmd == 1) ? White : c);
			NixDrawStr(x+8,y,L("open"));
			NixSetColor(White);
		}else{
			NixSetColor(White);
			NixDrawStr(x,y-12,_("administrieren"));
		}
		NixSetColor(White);
	}//else
		// title
		//NixDrawStr(x+10,y-3,name);
	NixDrawStr(x-25,y-52,name);
	msg_db_l(2);
}

void ModeWelcome::OnDraw()
{
	msg_db_r("Welcome.Draw", 1);
	NixSetColor(Black);
	NixDraw2D(r_id, NixTargetRect, 0.999999f);
	// logo
	float x=50;
	float y=5+MaxY*0.18f;
	float w=400;//512;
	float h=200;//256;
	float f=0.4f;//0.90f;
	color c=color(1,f,f,f);
	rect d=rect(x,x+w,y,y+h);
	NixSetTexture(tex_logo);
	NixSetColor(c);
	NixDraw2D(r_id, d, 0.999f);
	NixSetTexture(-1);
	NixDrawLineH(x, x + 720, y + 70, 0.99f);
	NixDrawLineH(x, x + 670, y + 73, 0.99f);

	// button stripe
	int dx = clampi(int((float)MaxX * 0.10f), 100, 200);
	int x1 = int((float)MaxX * 0.65f) - dx - 120;
	int y1 = int((float)MaxY * 0.80f) - 40;
	int y2 = y1 + 60;

	NixSetColor(color(1,0,0,0.2f));
	NixDrawRect(0, (float)MaxX, (float)y1, (float)y2, 0.5f);
	f=0.6f;
	NixSetColor(color(1,f,f,f));
	NixDrawLineH(0, MaxX, y1, 0);
	NixDrawLineH(0, MaxX, y2, 0);

	DrawIcon(x1     ,y1,0,0,_("Modell"));
	DrawIcon(x1+dx  ,y1,1,1,_("Material"));
	DrawIcon(x1+dx*2,y1,2,4,_("Welt"));
	DrawIcon(x1+dx*3,y1,3,5,_("Font"));
	DrawIcon(x1+dx*4,y1,4,6,_("Administration"));

	f=0.2f;
	NixSetColor(color(1,f,f,f));
	//NixSetFontColor(color(1,0.6f,0,0));
	NixDrawStr(0,MaxY-20,"(c) by MichiSoft TM 2012");
	NixSetColor(White);

	msg_db_l(1);
}

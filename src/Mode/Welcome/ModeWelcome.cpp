/*
 * ModeWelcome.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "../../Edward.h"
#include "ModeWelcome.h"
#include "../../lib/nix/nix.h"

ModeWelcome *mode_welcome = NULL;

ModeWelcome::ModeWelcome()
{
	name = "Welcome";
	parent = NULL;
	menu = NULL;
	multi_view = NULL;
	tex_icons = NixLoadTexture(HuiAppDirectoryStatic + "Data/icons/icons.tga");
	tex_logo = NixLoadTexture(HuiAppDirectoryStatic + "Data/edward.jpg");
}

ModeWelcome::~ModeWelcome()
{
}

void ModeWelcome::Start()
{
	mouse_over = mouse_over_cmd = -1;
}

void ModeWelcome::End()
{
}

void ModeWelcome::OnKeyDown()
{
}

void ModeWelcome::OnKeyUp()
{
}

void ModeWelcome::OnLeftButtonDown()
{
}

void ModeWelcome::OnLeftButtonUp()
{
	if (mouse_over == 0){
		if (mouse_over_cmd == 0)
			ed->NewModel();
		else
			ed->OpenModel();
	}else if (mouse_over == 1){
		if (mouse_over_cmd == 0)
			ed->NewMaterial();
		else
			ed->OpenMaterial();
	}else if (mouse_over == 2){
		if (mouse_over_cmd == 0)
			ed->NewWorld();
		else
			ed->OpenWorld();
	}else if (mouse_over == 3){
		if (mouse_over_cmd == 0)
			ed->NewFont();
		else
			ed->OpenFont();
	}else if (mouse_over == 4){
		//SetMode(ModeAdministration);
	}
}

void ModeWelcome::OnRightButtonUp()
{
}

void ModeWelcome::OnCommand(const string & id)
{
	if (id == "new")
		ed->NewModel();
	if (id == "open")
		ed->OpenModel();
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

void ModeWelcome::OnMiddleButtonDown()
{
}

void ModeWelcome::OnMiddleButtonUp()
{
}

void ModeWelcome::OnRightButtonDown()
{
}

void ModeWelcome::OnDataChange()
{
}


void ModeWelcome::DrawIcon(int x, int y, int index, const string &name)
{
	msg_db_r("DrawIcon", 2);
	rect s,d;
	y+=30;
	float rad=25;
	icon_rect[index] = irect(x-40,x+60,y-30,y+30);
	s.x1=(float)index*0.125f;
	s.x2=(float)index*0.125f+0.125f;
	s.y1=0;
	s.y2=1;

	// the icon itself
	d.x1=(float)x-rad;
	d.x2=(float)x+rad;
	d.y1=(float)y-rad;
	d.y2=(float)y+rad;
	NixSetAlpha(AlphaMaterial);
	color c=color((mouse_over==index)?0.3f:0.99f,1,1,1);
	NixDraw2D(tex_icons,c,s,d,0);
	NixSetAlpha(AlphaNone);

	x-=15;
	NixSetFontColor(color(1,0.6f,0.6f,0.6f));
	if (mouse_over == index){
		// operations to choose from
		if (index != 4){
			color c=color(1,0.5f,0.5f,0.6f); // unmarked: grayish
			NixSetFontColor((mouse_over_cmd == 0) ? White : c);
			NixDrawStr(x+17,y-25,L("new"));
			NixSetFontColor((mouse_over_cmd == 1) ? White : c);
			NixDrawStr(x+8,y,L("open"));
			NixSetFontColor(White);
		}else{
			NixSetFontColor(White);
			NixDrawStr(x,y-12,_("administrieren"));
		}
		NixSetFontColor(White);
	}//else
		// title
		//NixDrawStr(x+10,y-3,name);
	NixDrawStr(x-25,y-52,name);
	msg_db_l(2);
}



void ModeWelcome::DrawWin(int win, irect dest)
{
}

void ModeWelcome::Draw()
{
	msg_db_r("Welcome.Draw", 1);
	NixDraw2D(-1, Black, r01, NixTargetRect, 0.999999f);
	// logo
	float x=50;
	float y=5+MaxY*0.18f;
	float w=400;//512;
	float h=200;//256;
	float f=0.4f;//0.90f;
	color c=color(1,f,f,f);
	rect d=rect(x,x+w,y,y+h);
	NixDraw2D(tex_logo, c, r01, d, 0.999f);
	NixDrawLineH((int)x,(int)x+720,(int)y+70,c,0.99f);
	NixDrawLineH((int)x,(int)x+670,(int)y+73,c,0.99f);

	// button stripe
	int dx = clampi(int((float)MaxX * 0.10f), 100, 200);
	int x1 = int((float)MaxX * 0.65f) - dx - 120;
	int y1 = int((float)MaxY * 0.80f) - 40;
	int y2 = y1 + 60;

	NixDrawRect(0,(float)MaxX,(float)y1,(float)y2,color(1,0,0,0.2f),0.5f);
	f=0.6f;
	NixDrawLineH(0,MaxX,y1,color(1,f,f,f),0);
	NixDrawLineH(0,MaxX,y2,color(1,f,f,f),0);

	DrawIcon(x1     ,y1,0,_("Modell"));
	DrawIcon(x1+dx  ,y1,1,_("Material"));
	DrawIcon(x1+dx*2,y1,2,_("Welt"));
	DrawIcon(x1+dx*3,y1,3,_("Font"));
	DrawIcon(x1+dx*4,y1,4,_("Administration"));

	f=0.2f;
	NixSetFontColor(color(1,f,f,f));
	//NixSetFontColor(color(1,0.6f,0,0));
	NixDrawStr(0,MaxY-20,"(c) by MichiSoft TM 2012");
	NixSetFontColor(White);

	msg_db_l(1);
}

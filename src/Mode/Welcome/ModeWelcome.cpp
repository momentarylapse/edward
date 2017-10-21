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
	ModeBase("Welcome", NULL, NULL, "menu_nomode")
{
	tex_icons = nix::LoadTexture(app->directory_static + "Data/icons/icons.tga");
	tex_logo = nix::LoadTexture(app->directory_static + "Data/edward.jpg");
	mouse_over = -1;
	mouse_over_cmd = -1;
}

ModeWelcome::~ModeWelcome()
{
}

void ModeWelcome::onStart()
{
	hui::Toolbar *t = ed->toolbar[hui::TOOLBAR_TOP];
	t->reset();
	t->enable(true);
	t = ed->toolbar[hui::TOOLBAR_LEFT];
	t->reset();
	t->enable(false);

	mouse_over = mouse_over_cmd = -1;
}

void ModeWelcome::onEnd()
{
}

void ModeWelcome::onLeftButtonUp()
{
	if (mouse_over == 0){
		if (mouse_over_cmd == 0)
			mode_model->_new();
		else
			mode_model->open();
	}else if (mouse_over == 1){
		if (mouse_over_cmd == 0)
			mode_material->_new();
		else
			mode_material->open();
	}else if (mouse_over == 2){
		if (mouse_over_cmd == 0)
			mode_world->_new();
		else
			mode_world->open();
	}else if (mouse_over == 3){
		if (mouse_over_cmd == 0)
			mode_font->_new();
		else
			mode_font->open();
	}else if (mouse_over == 4){
		ed->setMode(mode_administration);
	}
}

void ModeWelcome::onCommand(const string & id)
{
	if (id == "new")
		mode_model->_new();
	if (id == "open")
		mode_model->open();
}

bool mouse_in_rect(rect r, int mx, int my)
{
	return ((mx > r.x1) && (mx < r.x2) && (my > r.y1) && (my < r.y2));
}

void ModeWelcome::onMouseMove()
{
	int mx = hui::GetEvent()->mx;
	int my = hui::GetEvent()->my;
	mouse_over = -1;
	mouse_over_cmd = -1;
	for (int i=0;i<5;i++)
		if (mouse_in_rect(icon_rect[i], mx, my)){
			mouse_over = i;
			mouse_over_cmd = (my < (icon_rect[i].y1 + icon_rect[i].y2) / 2) ? 0 : 1;
		}

	ed->forceRedraw();
}


void ModeWelcome::onUpdate(Observable *o, const string &message)
{
}

void ModeWelcome::DrawIcon(int x, int y, int index, int pic_index, const string &name)
{
	msg_db_f("DrawIcon", 2);
	rect s,d;
	y+=30;
	float rad=25;
	icon_rect[index] = rect(x-40,x+60,y-30,y+30);
	s.x1=(float)pic_index*0.125f;
	s.x2=(float)pic_index*0.125f+0.125f;
	s.y1=0;
	s.y2=1;

	// the icon itself
	d.x1=(float)x-rad;
	d.x2=(float)x+rad;
	d.y1=(float)y-rad;
	d.y2=(float)y+rad;
	nix::SetAlpha(AlphaMaterial);
	nix::SetTexture(tex_icons);
	nix::SetColor(color((mouse_over==index)?0.3f:0.99f,1,1,1));
	nix::Draw2D(s, d, 0);
	nix::SetTexture(NULL);
	nix::SetAlpha(AlphaNone);

	x-=15;
	nix::SetColor(color(1,0.6f,0.6f,0.6f));
	if (mouse_over == index){
		// operations to choose from
		if (index != 4){
			color c=color(1,0.5f,0.5f,0.6f); // unmarked: grayish
			nix::SetColor((mouse_over_cmd == 0) ? White : c);
			nix::DrawStr(x+17,y-25,L("", "new"));
			nix::SetColor((mouse_over_cmd == 1) ? White : c);
			nix::DrawStr(x+8,y,L("", "open"));
			nix::SetColor(White);
		}else{
			nix::SetColor(White);
			nix::DrawStr(x,y-12,_("administrieren"));
		}
		nix::SetColor(White);
	}//else
		// title
		//NixDrawStr(x+10,y-3,name);
	nix::DrawStr(x-25,y-52,name);
}

void ModeWelcome::onDraw()
{
	msg_db_f("Welcome.Draw", 1);
	nix::SetColor(Black);
	nix::Draw2D(r_id, nix::target_rect, 0.999999f);
	// logo
	float x=50;
	float y=5+nix::target_height*0.18f;
	float w=400;//512;
	float h=200;//256;
	float f=0.4f;//0.90f;
	color c=color(1,f,f,f);
	rect d=rect(x,x+w,y,y+h);
	nix::SetTexture(tex_logo);
	nix::SetColor(c);
	nix::Draw2D(r_id, d, 0.999f);
	nix::SetTexture(NULL);
	nix::DrawLineH(x, x + 720, y + 70, 0.99f);
	nix::DrawLineH(x, x + 670, y + 73, 0.99f);

	// button stripe
	int dx = clampi(int((float)nix::target_width * 0.10f), 100, 200);
	int x1 = int((float)nix::target_width * 0.65f) - dx - 120;
	int y1 = int((float)nix::target_height * 0.80f) - 40;
	int y2 = y1 + 60;

	nix::SetColor(color(1,0,0,0.2f));
	nix::DrawRect(0, (float)nix::target_width, (float)y1, (float)y2, 0.5f);
	f=0.6f;
	nix::SetColor(color(1,f,f,f));
	nix::DrawLineH(0, nix::target_width, y1, 0);
	nix::DrawLineH(0, nix::target_width, y2, 0);

	DrawIcon(x1     ,y1,0,0,_("Modell"));
	DrawIcon(x1+dx  ,y1,1,1,_("Material"));
	DrawIcon(x1+dx*2,y1,2,4,_("Welt"));
	DrawIcon(x1+dx*3,y1,3,5,_("Font"));
	DrawIcon(x1+dx*4,y1,4,6,_("Administration"));

	f=0.2f;
	nix::SetColor(color(1,f,f,f));
	//NixSetFontColor(color(1,0.6f,0,0));
	nix::DrawStr(0,nix::target_height-20,"(c) by MichiSoft TM 2015");
	nix::SetColor(White);
}

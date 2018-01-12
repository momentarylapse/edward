/*
 * GameIniData.cpp
 *
 *  Created on: 02.03.2013
 *      Author: michi
 */

#include "GameIniData.h"


void GameIniData::Load(const string &dir)
{
	File *f = FileOpen(dir + "game.ini");
	f->ReadComment();
	DefScript = f->ReadStr();
	f->ReadComment();
	DefWorld = f->ReadStr();
	f->ReadComment();
	SecondWorld = f->ReadStr();
	f->ReadComment();
	DefMaterial = f->ReadStr();
	f->ReadComment();
	DefFont = f->ReadStr();
	delete(f);
}

void GameIniData::Save(const string &dir)
{
	File *f = FileCreate(dir + "game.ini");
	f->WriteStr("// Main Script");
	f->WriteStr(DefScript);
	f->WriteStr("// Default World");
	f->WriteStr(DefWorld);
	f->WriteStr("// Second World");
	f->WriteStr(SecondWorld);
	f->WriteStr("// Default Material");
	f->WriteStr(DefMaterial);
	f->WriteStr("// Default Font");
	f->WriteStr(DefFont);
	f->WriteStr("#");
	delete(f);
}


void GameIniData::reset()
{
	DefScript = "";
	DefWorld = "";
	SecondWorld = "";
	DefMaterial = "";
	DefFont = "";
}

/*
 * GameIniData.cpp
 *
 *  Created on: 02.03.2013
 *      Author: michi
 */

#include "GameIniData.h"


void GameIniData::Load(const string &dir)
{
	msg_db_r("LoadGameIni",5);
	CFile *f = OpenFile(dir + "game.ini");
	DefScript = f->ReadStrC();
	DefWorld = f->ReadStrC();
	SecondWorld = f->ReadStrC();
	DefMaterial = f->ReadStrC();
	DefFont = f->ReadStrC();
	DefTextureFxMetal = f->ReadStrC();
	delete(f);
	msg_db_l(5);
}

void GameIniData::Save(const string &dir)
{
	msg_db_r("SaveGameIni",5);
	CFile *f = CreateFile(dir + "game.ini");
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
	f->WriteStr("// Texture FX-metal");
	f->WriteStr(DefTextureFxMetal);
	f->WriteStr("#");
	delete(f);
	msg_db_l(5);
}


void GameIniData::reset()
{
	DefScript = "";
	DefWorld = "";
	SecondWorld = "";
	DefMaterial = "";
	DefFont = "";
	DefTextureFxMetal = "";
}

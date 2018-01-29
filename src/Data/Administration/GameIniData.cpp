/*
 * GameIniData.cpp
 *
 *  Created on: 02.03.2013
 *      Author: michi
 */

#include "GameIniData.h"


void GameIniData::Load(const string &dir)
{
	try{
		File *f = FileOpenText(dir + "game.ini");
		f->read_comment();
		DefScript = f->read_str();
		f->read_comment();
		DefWorld = f->read_str();
		f->read_comment();
		SecondWorld = f->read_str();
		f->read_comment();
		DefMaterial = f->read_str();
		f->read_comment();
		DefFont = f->read_str();
		delete(f);
	}catch(...){}
}

void GameIniData::Save(const string &dir)
{
	try{
		File *f = FileCreateText(dir + "game.ini");
		f->write_comment("// Main Script");
		f->write_str(DefScript);
		f->write_comment("// Default World");
		f->write_str(DefWorld);
		f->write_comment("// Second World");
		f->write_str(SecondWorld);
		f->write_comment("// Default Material");
		f->write_str(DefMaterial);
		f->write_comment("// Default Font");
		f->write_str(DefFont);
		f->write_str("#");
		delete(f);
	}catch(...){}
}


void GameIniData::reset()
{
	DefScript = "";
	DefWorld = "";
	SecondWorld = "";
	DefMaterial = "";
	DefFont = "";
}

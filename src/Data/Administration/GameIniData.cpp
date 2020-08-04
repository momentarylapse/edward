/*
 * GameIniData.cpp
 *
 *  Created on: 02.03.2013
 *      Author: michi
 */

#include "GameIniData.h"


void GameIniData::load(const Path &dir) {
	try {
		File *f = FileOpenText(dir << "game.ini");
		f->read_comment();
		default_script = f->read_str();
		f->read_comment();
		default_world = f->read_str();
		f->read_comment();
		second_world = f->read_str();
		f->read_comment();
		default_material = f->read_str();
		f->read_comment();
		default_font = f->read_str();
		delete f;
	} catch(...) {}
}

void GameIniData::save(const Path &dir) {
	try {
		File *f = FileCreateText(dir << "game.ini");
		f->write_comment("// Main Script");
		f->write_str(default_script.str());
		f->write_comment("// Default World");
		f->write_str(default_world.str());
		f->write_comment("// Second World");
		f->write_str(second_world.str());
		f->write_comment("// Default Material");
		f->write_str(default_material.str());
		f->write_comment("// Default Font");
		f->write_str(default_font.str());
		f->write_str("#");
		delete f;
	} catch(...) {}
}


void GameIniData::reset() {
	default_script = "";
	default_world = "";
	second_world = "";
	default_material = "";
	default_font = "";
}

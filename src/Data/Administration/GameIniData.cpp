/*
 * GameIniData.cpp
 *
 *  Created on: 02.03.2013
 *      Author: michi
 */

#include "GameIniData.h"


const string GameIniData::ID_SCRIPT = "default-script";
const string GameIniData::ID_WORLD = "default-world";
const string GameIniData::ID_WORLD2 = "second-world";
const string GameIniData::ID_MATERIAL = "default-material";
const string GameIniData::ID_FONT = "default-font";

void GameIniData::load(const Path &dir) {
	hui::Configuration::load(dir  << "game.ini");
}

void GameIniData::save(const Path &dir) {
	hui::Configuration::save(dir  << "game.ini");
}

Path GameIniData::default_font() const {
	return get_str(ID_FONT, "");
}

Path GameIniData::default_script() const {
	return get_str(ID_SCRIPT, "");
}

Path GameIniData::default_material() const {
	return get_str(ID_MATERIAL, "");
}

Path GameIniData::default_world() const {
	return get_str(ID_WORLD, "");
}

Path GameIniData::second_world() const {
	return get_str(ID_WORLD2, "");
}

void GameIniData::reset() {
	map.clear();
	//set_str(ID_SCRIPT, "");
	//set_str(ID_WORLD, "");
	//set_str(ID_WORLD2, "");
	//set_str(ID_MATERIAL, "");
	//set_str(ID_FONT, "");
}

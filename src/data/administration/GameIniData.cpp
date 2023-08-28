/*
 * GameIniData.cpp
 *
 *  Created on: 02.03.2013
 *      Author: michi
 */

#include "GameIniData.h"
#include "../../lib/any/any.h"


const string GameIniData::ID_SCRIPT = "default.main-script";
const string GameIniData::ID_WORLD = "default.world";
const string GameIniData::ID_WORLD2 = "default.second-world";
const string GameIniData::ID_MATERIAL = "default.material";
const string GameIniData::ID_FONT = "default.font";
const string GameIniData::ID_SCREEN_MODE = "screen.mode";
const string GameIniData::ID_SCREEN_WIDTH = "screen.width";
const string GameIniData::ID_SCREEN_HEIGHT = "screen.height";
const string GameIniData::ID_RENDER_PATH = "renderer.path";
const string GameIniData::ID_RENDERER_FRAMERATE = "renderer.target-framerate";
const string GameIniData::ID_RESOLUTION_SCALE_MIN = "renderer.resolution-scale-min";
const string GameIniData::ID_SHADOW_BOXSIZE = "shadow.boxsize";
const string GameIniData::ID_SHADOW_RESOLUTION = "shadow.resolution";
const string GameIniData::ID_DEBUG_LEVEL = "debug.level";
const string GameIniData::ID_DEBUG_SCRIPTS1 = "debug.scripts1";
const string GameIniData::ID_DEBUG_SCRIPTS2 = "debug.scripts2";

void GameIniData::load(const Path &dir) {
	Configuration::load(dir | "game.ini");
}

void GameIniData::save(const Path &dir) {
	Configuration::save(dir | "game.ini");
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

void GameIniData::reset_default() {
	map.clear();
	set_str(ID_SCRIPT, "");
	set_str(ID_WORLD, "");
	set_str(ID_WORLD2, "");
	set_str(ID_MATERIAL, "");
	set_str(ID_FONT, "");
	set_str_array("default.additional-scripts", {});

	set_str(ID_SCREEN_MODE, "windowed");
	set_int(ID_SCREEN_WIDTH, 1920);
	set_int(ID_SCREEN_HEIGHT, 1080);

	set_str(ID_RENDER_PATH, "forward");
	set_int(ID_RENDERER_FRAMERATE, 60);
	set_float(ID_RESOLUTION_SCALE_MIN, 0.5f);

	set_int(ID_DEBUG_LEVEL, 1);
	set_str_array(ID_DEBUG_SCRIPTS1, {"debug/gui-stats.kaba", "debug/wireframe.kaba"});
	set_str_array(ID_DEBUG_SCRIPTS2, {"debug/hdr.kaba", "debug/gbuffer.kaba", "debug/shadows.kaba"});

	set_float(ID_SHADOW_BOXSIZE, 2000);
	set_int(ID_SHADOW_RESOLUTION, 2048);
}

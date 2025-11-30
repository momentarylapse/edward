/*
 * GameIniData.cpp
 *
 *  Created on: 02.03.2013
 *      Author: michi
 */

#include "GameIniData.h"
#include <y/EngineData.h>
#include <lib/any/any.h>



void GameIniData::load(const Path &dir) {
	Configuration::load(dir | "game.ini");
}

void GameIniData::save(const Path &dir) {
	Configuration::save(dir | "game.ini");
}

Path GameIniData::default_font() const {
	return get_str(ID_DEFAULT_FONT, "");
}

Path GameIniData::default_script() const {
	return get_str(ID_DEFAULT_SCRIPT, "");
}

Path GameIniData::default_material() const {
	return get_str(ID_DEFAULT_MATERIAL, "");
}

Path GameIniData::default_world() const {
	return get_str(ID_DEFAULT_WORLD, "");
}

Path GameIniData::second_world() const {
	return get_str(ID_DEFAULT_WORLD2, "");
}

void GameIniData::reset_default() {
	map.clear();
	set_int(ID_API_VERSION, EngineData::CURRENT_API_VERSION);
	set_str(ID_DEFAULT_SCRIPT, "");
	set_str(ID_DEFAULT_WORLD, "");
	set_str(ID_DEFAULT_WORLD2, "");
	set_str(ID_DEFAULT_MATERIAL, "");
	set_str(ID_DEFAULT_FONT, "");
	set_str_array("default.additional-scripts", {});

	set_str(ID_SCREEN_MODE, "windowed");
	set_int(ID_SCREEN_WIDTH, 1920);
	set_int(ID_SCREEN_HEIGHT, 1080);

	set_str(ID_RENDERER_PATH, "forward");
	set_int(ID_RENDERER_TARGET_FRAMERATE, 60);
	set_float(ID_RESOLUTION_SCALE_MIN, 0.5f);

	set_str(ID_RENDERER_LIGHTING, "pbr");
	set_str(ID_RENDERER_LIGHT_SOURCES, "default");

	set_int(ID_DEBUG_LEVEL, 1);
	set_str_array(ID_DEBUG_SCRIPTS1, {"debug/stats.kaba", "debug/wireframe.kaba"});
	set_str_array(ID_DEBUG_SCRIPTS2, {"debug/hdr.kaba", "debug/gbuffer.kaba", "debug/shadows.kaba"});

	set_float(ID_SHADOW_BOXSIZE, 2000);
	set_int(ID_SHADOW_RESOLUTION, 2048);
	set_str(ID_SHADOW_QUALITY, "pcf-hardening");

	set_int(ID_CUBEMAP_RESOLUTION, 64);
	set_int(ID_CUBEMAP_UPDATE_RATE, 9);
}

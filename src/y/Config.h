/*
 * Config.h
 *
 *  Created on: 07.08.2020
 *      Author: michi
 */

#pragma once


#include <lib/base/base.h>
#include <lib/os/config.h>
#include <lib/os/path.h>

enum class AntialiasingMethod {
	NONE,
	MSAA,
	TAA
};

enum class ScreenMode {
	Windowed,
	Fullscreen,
	WindowedFullscreen
};

struct DigestedConfig {
	string api_version = "0";
	int debug_level = 0;
	string main_script;
	string default_world;
	string second_world;
	string default_material;
	string default_font;
	Path game_dir;
	AntialiasingMethod antialiasing_method = AntialiasingMethod::NONE;
	bool allow_rtx = true;
	Array<string> additional_scripts;
	bool ignore_missing_files = false;

	int screen_width = 0;
	int screen_height = 0;
	ScreenMode screen_mode;

	float resolution_scale_min = 0;
	float resolution_scale_max = 0;
	string resolution_scale_filter;
	float target_framerate = 0;
	float min_framerate = 0;
	bool uncapped_framerate = false;
	float ambient_occlusion_radius = 0;

	float shadow_box_size;
	int shadow_resolution;
	string shadow_quality;
	string light_sources;
	string lighting;
	string renderer_path;

	int cubemap_resolution = 64;
	int cubemap_update_rate = 0;
};
extern DigestedConfig config;

class RawConfig : public Configuration {
public:
	RawConfig();
	void load(const Array<string> &arg);
	DigestedConfig digest() const;

	static const string ID_API_VERSION;
	static const string ID_DEFAULT_SCRIPT;
	static const string ID_DEFAULT_WORLD;
	static const string ID_DEFAULT_WORLD2;
	static const string ID_DEFAULT_MATERIAL;
	static const string ID_DEFAULT_FONT;
	static const string ID_SCREEN_MODE;
	static const string ID_SCREEN_WIDTH;
	static const string ID_SCREEN_HEIGHT;
	static const string ID_RENDERER_PATH;
	static const string ID_RENDERER_TARGET_FRAMERATE;
	static const string ID_RENDERER_MIN_FRAMERATE;
	static const string ID_RENDERER_ANTIALIASING;
	static const string ID_RENDERER_LIGHTING;
	static const string ID_RENDERER_LIGHT_SOURCES;
	static const string ID_RESOLUTION_SCALE_MIN;
	static const string ID_RESOLUTION_SCALE_MAX;
	static const string ID_RESOLUTION_SCALE_FILTER;
	static const string ID_SHADOW_BOXSIZE;
	static const string ID_SHADOW_RESOLUTION;
	static const string ID_SHADOW_QUALITY;
	static const string ID_CUBEMAP_RESOLUTION;
	static const string ID_CUBEMAP_UPDATE_RATE;
	static const string ID_DEBUG_LEVEL;
	static const string ID_DEBUG_SCRIPTS1;
	static const string ID_DEBUG_SCRIPTS2;
};

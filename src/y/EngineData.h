/*
 * EngineData.h
 *
 *  Created on: 16.08.2020
 *      Author: michi
 */

#pragma once


#include "../lib/base/base.h"
#include "../lib/file/path.h"


namespace Gui {
	class Font;
}


class TargetRenderer;
class Renderer;
class RenderPath;

class EngineData {
public:
	EngineData();

	string app_name, version;
	bool debug, show_timings, console_enabled, wire_mode;
	float detail_level;
	float detail_factor_inv;
	int shadow_level;
	bool shadow_lower_detail;

	bool ignore_missing_files;

	int multisampling;
	bool CullingEnabled, SortingEnabled, ZBufferEnabled;
	bool resetting_game;
	Gui::Font *default_font;
	Path initial_world_file, second_world_file;
	bool physics_enabled, collisions_enabled;
	int mirror_level_max;

	int num_real_col_tests;

	float fps_max, fps_min;
	float time_scale, elapsed, elapsed_rt;

	// output rendering/frame buffer resolution (might be smaller than the physical screen resolution)
	int width, height;


	// dynamic resolution scaling
	float resolution_scale_x = 1.0f;
	float resolution_scale_y = 1.0f;


	// the "real world" aspect ratio (cm/cm) of the output image (screen or window)
	float physical_aspect_ratio;

	bool first_frame;
	bool game_running;

	bool file_errors_are_critical;

	void set_dirs(const Path &texture_dir, const Path &map_dir, const Path &object_dir, const Path &sound_dir, const Path &script_dir, const Path &material_dir, const Path &font_dir);

	Path map_dir, sound_dir, script_dir, object_dir, texture_dir, shader_dir, material_dir, font_dir;

	TargetRenderer *window_renderer;
	Renderer *gui_renderer;
	Renderer *hdr_renderer;
	Renderer *post_processor;
	RenderPath *render_path;
};
extern EngineData engine;




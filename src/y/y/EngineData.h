/*
 * EngineData.h
 *
 *  Created on: 16.08.2020
 *      Author: michi
 */

#pragma once


#include <lib/base/base.h>
#include <lib/os/path.h>
#include <lib/ygraphics/graphics-fwd.h>


namespace gui {
	class Font;
}

namespace yrenderer {
	class Context;
	class TargetRenderer;
	class Renderer;
	class MaterialManager;
	class WindowRenderer;
	class RegionRenderer;
	class RenderTask;
}


class ResourceManager;
class ModelManager;
class FullCameraRenderer;

class EngineData {
public:
	EngineData();
	void set_context(yrenderer::Context *ctx, ResourceManager *rm);

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
	gui::Font *default_font;
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

	bool end_requested = false;
	void exit();

	bool file_errors_are_critical;

	void set_dirs(const Path &texture_dir, const Path &map_dir, const Path &object_dir, const Path &sound_dir, const Path &script_dir, const Path &material_dir, const Path &font_dir);

	Path map_dir, sound_dir, script_dir, object_dir, texture_dir, shader_dir, material_dir, font_dir;

	yrenderer::Context* context;
	ResourceManager *resource_manager;

	yrenderer::WindowRenderer* window_renderer;
	yrenderer::Renderer* gui_renderer;
	yrenderer::RegionRenderer* region_renderer;
	Array<FullCameraRenderer*> camera_renderers;

	Array<yrenderer::RenderTask*> render_tasks;
	void add_render_task(yrenderer::RenderTask* task, int priority);
};
extern EngineData engine;




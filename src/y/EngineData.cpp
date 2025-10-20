/*
 * EngineData.cpp
 *
 *  Created on: 16.08.2020
 *      Author: michi
 */


#include "EngineData.h"
#include <lib/config.h>
#include <lib/base/sort.h>
#include <lib/nix/nix.h>
#include <lib/kaba/kaba.h>
#include <lib/yrenderer/Renderer.h>

#include <helper/ResourceManager.h>
#include <lib/yrenderer/ShaderManager.h>
#include <lib/yrenderer/TextureManager.h>

EngineData engine;

EngineData::EngineData() {
	context = nullptr;
	resource_manager = nullptr;

	default_font = nullptr;

	ZBufferEnabled = true;
	CullingEnabled = false;
	SortingEnabled = false;
	console_enabled = false;
	show_timings = false;
	debug = false;
	ignore_missing_files = false;

	shadow_lower_detail = false;
	shadow_level = 0;

	fps_max = 60;
	fps_min = 15;

	width = 0;
	height = 0;
	window_renderer = nullptr;
	gui_renderer = nullptr;
	region_renderer = nullptr;

	detail_level = 100;
	detail_factor_inv = 1.0f;
	mirror_level_max = 1;
	multisampling = 1;

	wire_mode = false;
	elapsed = elapsed_rt = 0;

	time_scale = 1.0f;
	file_errors_are_critical = false;
	physical_aspect_ratio = 1.333f;

	num_real_col_tests = 0;
	resetting_game = false;
	first_frame = false;
	game_running = false;
}

void EngineData::set_context(yrenderer::Context *ctx, ResourceManager *rm) {
	context = ctx;
	resource_manager = rm;
}

void EngineData::set_dirs(const Path &_texture_dir, const Path &_map_dir, const Path &_object_dir, const Path &_sound_dir, const Path &_script_dir, const Path &_material_dir, const Path &_font_dir) {
	texture_dir = _texture_dir;
	shader_dir = _material_dir;
	map_dir = _map_dir;
	object_dir = _object_dir;
	sound_dir = _sound_dir;
	script_dir = _script_dir;
	material_dir = _material_dir;
	font_dir = _font_dir;
	kaba::config.directory = _script_dir;
}

void EngineData::exit() {
	end_requested = true;
}

void EngineData::add_render_task(yrenderer::RenderTask* task, int priority) {
	task->_priority = priority;
	render_tasks.add(task);
	base::inplace_sort(render_tasks, [](yrenderer::RenderTask *a, yrenderer::RenderTask *b) {
		return a->_priority <= b->_priority;
	});
}




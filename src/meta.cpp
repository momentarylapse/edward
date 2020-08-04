/*----------------------------------------------------------------------------*\
| Meta                                                                         |
| -> administration of animations, models, items, materials etc                |
| -> centralization of game data                                               |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
|  - paths get completed with the belonging root-directory of the file type    |
|    (model,item)                                                              |
|  - independent models                                                        |
|     -> equal loading commands create new instances                           |
|     -> equal loading commands copy existing models                           |
|         -> databases of original and copied models                           |
|         -> some data is referenced (skin...)                                 |
|         -> additionally created: effects, absolute physical data,...         |
|     -> each object has its own model                                         |
|  - independent items (managed by CMeta)                                      |
|     -> new items additionally saved as an "original item"                    |
|     -> an array of pointers points to each item                              |
|     -> each item has its unique ID (index in the array) for networking       |
|  - materials stay alive forever, just one instance                           |
|                                                                              |
| last updated: 2009.12.09 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#include "lib/file/file.h"
#include "meta.h"
//#include "lib/vulkan/vulkan.h"
#include "lib/nix/nix.h"
#include "lib/kaba/kaba.h"
#ifdef _X_ALLOW_X_
#include "world/model.h"
#if 0
#include "world/model_manager.h"
#endif
#include "world/material.h"
#include "world/world.h"
#include "world/camera.h"
#if 0
#include "gui/gui.h"
#include "gui/font.h"
#include "fx/fx.h"
#endif
#else // for use in Edward
#include "x/material.h"
#include "x/model.h"
#include "x/ModelManager.h"
#include "x/font.h"
#endif




// game configuration

EngineData engine;


Array<XContainer*> meta_delete_stuff_list;

bool AllowXContainer = true;




EngineData::EngineData() {
	default_font = NULL;

	ZBufferEnabled = true;
	CullingEnabled = false;
	SortingEnabled = false;
	console_enabled = false;
	show_timings = false;
	debug = false;

	shadow_light = 0;
	shadow_lower_detail = false;
	shadow_color = color(0.5f, 0, 0, 0);
	shadow_level = 0;

	fps_max = 60;
	fps_min = 15;

	detail_level = 100;
	detail_factor_inv = 1.0f;
	mirror_level_max = 1;
	multisampling = 1;

	wire_mode = false;
	physics_enabled = false;
	collisions_enabled = false;
	elapsed = elapsed_rt = 0;

	time_scale = 1.0f;
	file_errors_are_critical = false;

	num_real_col_tests = 0;
	resetting_game = false;
	first_frame = false;
	game_running = false;
}


void MetaReset() {
	meta_delete_stuff_list.clear();

#if 0
	if (Gui::Fonts.num > 0)
		Engine.DefaultFont = Gui::Fonts[0];
#endif
	engine.shadow_light = 0;
	engine.shadow_lower_detail = false;
	engine.shadow_color = color(0.5f, 0, 0, 0);
}

void EngineData::set_dirs(const Path &texture_dir, const Path &_map_dir, const Path &_object_dir, const Path &_sound_dir, const Path &_script_dir, const Path &material_dir, const Path &font_dir) {
#if LIB_HAS_VULKAN
	vulkan::Texture::directory = texture_dir;
	vulkan::Shader::directory = material_dir;
#endif
	nix::texture_dir = texture_dir;
	nix::shader_dir = material_dir;

	map_dir = _map_dir;
	object_dir = _object_dir;
	sound_dir = _sound_dir;
	script_dir = _script_dir;
	MaterialDir = material_dir;
#if 0
	Gui::FontDir = font_dir;
#endif
	Kaba::config.directory = script_dir;
}

void MetaCalcMove() {
/*	for (int i=0;i<NumTextures;i++)
		NixTextureVideoMove(Texture[i],Elapsed);*/
	msg_todo("MetaCalcMove...");
	
	engine.detail_factor_inv = 100.0f/(float)engine.detail_level;
}


void MetaDeleteLater(XContainer *p) {
	meta_delete_stuff_list.add(p);
}

void MetaDeleteSelection() {
	for (auto *p: meta_delete_stuff_list)
		delete p;
	meta_delete_stuff_list.clear();
}

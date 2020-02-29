/*----------------------------------------------------------------------------*\
| Meta                                                                         |
| -> administration of animations, models, items, materials etc                |
| -> centralization of game data                                               |
|                                                                              |
| last updated: 2009.11.22 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#if !defined(META_H__INCLUDED_)
#define META_H__INCLUDED_

#include "lib/base/base.h"
#include "lib/image/color.h"
#include "lib/hui/hui.h"

class Model;
class Object;
class Terrain;
class Light;




namespace Gui {
	class Font;
}

class EngineData {
public:
	EngineData();

	string app_name, version;
	bool debug, show_timings, console_enabled, wire_mode;
	float detail_level;
	float detail_factor_inv;
	int shadow_level;
	bool shadow_lower_detail;
	Light *shadow_light;
	color shadow_color;
	
	int multisampling;
	bool CullingEnabled, SortingEnabled, ZBufferEnabled;
	bool resetting_game;
	Gui::Font *default_font;
	string initial_world_file, second_world_file;
	bool physics_enabled, collisions_enabled;
	int mirror_level_max;
	
	int num_real_col_tests;
	
	float fps_max, fps_min;
	float time_scale, elapsed, elapsed_rt;

	bool first_frame;
	bool game_running;

	bool file_errors_are_critical;

	void set_dirs(const string &texture_dir, const string &map_dir, const string &object_dir, const string &sound_dir, const string &script_dir, const string &material_dir, const string &font_dir);

	string map_dir, sound_dir, script_dir, object_dir;
};
extern EngineData engine;



class TemplateDataScriptVariable {
public:
	string name, value;
};

class TemplateDataScript {
public:
	string filename;
	Array<TemplateDataScriptVariable> variables;
};


typedef void str_float_func(const string&,float);


void MetaReset();
void MetaCalcMove();



enum {
	ScriptLocationCalcMovePrae,
	ScriptLocationCalcMovePost,
	ScriptLocationRenderPrae,
	ScriptLocationRenderPost,
	ScriptLocationGetInputPrae,
	ScriptLocationGetInputPost,
	ScriptLocationNetworkSend,
	ScriptLocationNetworkRecieve,
	ScriptLocationNetworkAddClient,
	ScriptLocationNetworkRemoveClient,
	ScriptLocationWorldInit,
	ScriptLocationWorldDelete,
	ScriptLocationOnKeyDown,
	ScriptLocationOnKeyUp,
	ScriptLocationOnLeftButtonDown,
	ScriptLocationOnLeftButtonUp,
	ScriptLocationOnMiddleButtonDown,
	ScriptLocationOnMiddleButtonUp,
	ScriptLocationOnRightButtonDown,
	ScriptLocationOnRightButtonUp,
};



class XContainer : public hui::EventHandler {
public:
	virtual ~XContainer(){}
	virtual void _cdecl on_iterate(float dt){}
	virtual void _cdecl on_init(){}
	virtual void _cdecl on_delete(){}
};


void _cdecl MetaDeleteLater(XContainer *p);
void _cdecl MetaDeleteSelection();

extern bool AllowXContainer;

#define xcon_reg(var, array) \
	if (AllowXContainer) \
		(array).add(var);

#define xcon_unreg(var, array) \
	if (AllowXContainer) \
		for (int i=0;i<(array).num;i++) \
			if ((array)[i] == var) \
				(array).erase(i);

#define xcon_del(array) \
	for (int i=(array).num-1; i>=0; i--) \
		delete((array)[i]); \
	(array).clear();

#endif


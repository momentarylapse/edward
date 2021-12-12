/*
 * Edward.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef EDWARD_H_
#define EDWARD_H_

#include "lib/hui/hui.h"
#include "lib/base/map.h"
#include "Mode/Mode.h"
#include "Stuff/Progress.h"
#include "Stuff/PluginManager.h"

namespace nix{
	class Texture;
};
namespace MultiView{
	class MultiView;
};

class ModeModel;
class ModeMaterial;
class ModeWorld;
class ModeFont;
class ModeAdministration;



// multiview data
enum {
	MVD_MODEL_SURFACE,
	MVD_MODEL_VERTEX,
	MVD_MODEL_SKIN_VERTEX,
	MVD_MODEL_POLYGON,
	MVD_MODEL_EDGE,
	MVD_MODEL_BALL,
	MVD_MODEL_CYLINDER,
	MVD_MODEL_POLYHEDRON,
	MVD_SKELETON_BONE,
	MVD_WORLD_OBJECT,
	MVD_WORLD_TERRAIN,
	MVD_WORLD_TERRAIN_VERTEX,
	MVD_WORLD_LIGHT,
	MVD_WORLD_LINK,
	MVD_WORLD_SCRIPT,
	MVD_WORLD_CAMERA,
	MVD_WORLD_CAM_POINT,
	MVD_WORLD_CAM_POINT_VEL,
};


class Edward : public hui::Window {
public:
	Edward(Array<string> arg);
	~Edward() override;

	bool handle_arguments(Array<string> arg);
	void load_key_codes();

	void idle_function();

	void on_about();
	void on_send_bug_report();
	void set_mode(ModeBase *m);

	void on_command(const string &id);
	void on_close();

	ModeBase *get_mode(int preferred_type);
	bool universal_new(int preferred_type);
	bool universal_open(int preferred_type);

	virtual void _cdecl on_draw_gl();
	virtual void _cdecl on_key_down() override;
	virtual void _cdecl on_key_up() override;
	virtual void _cdecl on_mouse_move() override;
	virtual void _cdecl on_mouse_wheel() override;
	virtual void _cdecl on_mouse_enter() override;
	virtual void _cdecl on_mouse_leave() override;
	virtual void _cdecl on_left_button_down() override;
	virtual void _cdecl on_left_button_up() override;
	virtual void _cdecl on_middle_button_down() override;
	virtual void _cdecl on_middle_button_up() override;
	virtual void _cdecl on_right_button_down() override;
	virtual void _cdecl on_right_button_up() override;
	void on_event();
	void on_abort_creation_mode();

	void set_message(const string &message);
	void remove_message();
	void error_box(const string &message);
	Array<string> message_str;

	void on_execute_plugin();

	bool allow_termination();

	void update_menu();
	void optimize_current_view();

	ModeBase *mode_none;
	ModeModel* mode_model;
	ModeMaterial* mode_material;
	ModeWorld* mode_world;
	ModeFont* mode_font;
	ModeAdministration* mode_admin;
	ModeBase *cur_mode;
	Array<ModeBase*> mode_queue;

	MultiView::MultiView *multi_view_2d;
	MultiView::MultiView *multi_view_3d;

	Progress *progress;

	PluginManager *plugins;

	hui::Panel *side_panel, *bottom_panel;
	void set_side_panel(hui::Panel *panel);
	void set_bottom_panel(hui::Panel *panel);


	Map<nix::Texture*, string> icon_image;
	string get_tex_image(nix::Texture *tex);

	hui::Timer timer;
};

extern Edward *ed;

class EdwardApp : public hui::Application
{
public:
	EdwardApp();
	virtual bool on_startup(const Array<string> &arg) override;

};

extern EdwardApp *app;

#endif /* EDWARD_H_ */

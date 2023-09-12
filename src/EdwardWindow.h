/*
 * EdwardWindow.h
 *
 *  Created on: 10 Sept 2023
 *      Author: michi
 */

#ifndef SRC_EDWARDWINDOW_H_
#define SRC_EDWARDWINDOW_H_

#include "lib/hui/hui.h"
#include "lib/base/map.h"
#include "lib/os/time.h"
#include "lib/pattern/Observable.h"

namespace nix {
	class Texture;
	class Context;
};
namespace MultiView {
	class MultiView;
};

class ModeBase;
class ModeModel;
class ModeMaterial;
class ModeWorld;
class ModeFont;
class ModeAdministration;
class Storage;
class Progress;
class ResourceManager;
class DrawingHelper;



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


class EdwardWindow : public obs::Node<hui::Window> {
public:
	EdwardWindow();
	~EdwardWindow() override;

	obs::sink in_data_selection_changed;
	obs::sink in_data_changed;
	obs::sink in_action_failed;
	obs::sink in_saved;

	void load_key_codes();

	void idle_function();

	void on_about();
	void on_send_bug_report();
	void set_mode(ModeBase *m);
	void set_mode_now(ModeBase *m);

	void on_command(const string &id);
	void on_close();

	ModeBase *get_mode(int type);
	void universal_new(int type);
	void universal_open(int preferred_type);
	void universal_edit(int type, const Path &filename, bool relative_path);

	void on_draw_gl();
	void on_realize_gl();
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

	base::future<void> allow_termination();

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
	ModeBase *find_mode_base(const string &name);
	template<class M>
	M *find_mode(const string &name) {
		return static_cast<M*>(find_mode_base(name));
	}

	nix::Context *gl;
	ResourceManager *resource_manager;
	DrawingHelper *drawing_helper;

	Storage *storage;
	MultiView::MultiView *multi_view_2d;
	MultiView::MultiView *multi_view_3d;

	Progress *progress;

	shared<hui::Panel> side_panel, bottom_panel;
	void set_side_panel(shared<hui::Panel> panel);
	void set_bottom_panel(shared<hui::Panel> panel);


	base::map<nix::Texture*, string> icon_image;
	string get_tex_image(nix::Texture *tex);

	os::Timer timer;

	base::promise<void> promise_started;
};

#endif /* SRC_EDWARDWINDOW_H_ */

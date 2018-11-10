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
#include "Stuff/Observer.h"
#include "Stuff/Progress.h"
#include "Stuff/PluginManager.h"

namespace nix{
	class Texture;
};
namespace MultiView{
	class MultiView;
};



// file types
enum{
	FD_MODEL,
	FD_TEXTURE,
	FD_SOUND,
	FD_MATERIAL,
	FD_TERRAIN,
	FD_WORLD,
	FD_SHADERFILE,
	FD_FONT,
	FD_SCRIPT,
	FD_CAMERAFLIGHT,
	FD_FILE, // any file
	NUM_FDS
};

// multiview data
enum{
	MVD_MODEL_SURFACE,
	MVD_MODEL_VERTEX,
	MVD_MODEL_SKIN_VERTEX,
	MVD_MODEL_POLYGON,
	MVD_MODEL_EDGE,
	MVD_MODEL_BALL,
	MVD_MODEL_POLYHEDRON,
	MVD_SKELETON_BONE,
	MVD_WORLD_OBJECT,
	MVD_WORLD_TERRAIN,
	MVD_WORLD_TERRAIN_VERTEX,
	MVD_WORLD_CAM_POINT,
	MVD_WORLD_CAM_POINT_VEL,
};


void read_color_4i(File *f, int *c);
void write_color_4i(File *f, int *c);
void read_color_argb(File *f, color &c);
void write_color_argb(File *f, const color &c);
void read_color_rgba(File *f, color &c);
void write_color_rgba(File *f, const color &c);
void read_color_3i(File *f, color &c);
void write_color_3i(File *f, const color &c);
color i42c(int *c);

class Edward : public Observer, public hui::Window
{
public:
	Edward(Array<string> arg);
	virtual ~Edward();

	virtual void on_destroy() override;

	bool handle_arguments(Array<string> arg);
	void load_key_codes();
	void exit();

	void idle_function();

	void on_about();
	void on_send_bug_report();
	void set_mode(ModeBase *m);

	void on_update(Observable *o, const string &message);
	void on_command(const string &id);
	void on_close();

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

	void set_root_directory(const string &directory, bool compact_mode = false);
	void update_dialog_dir(int kind);
	void make_dirs(const string &original_dir, bool as_root_dir = false);
	string get_root_dir(int kind);

	bool file_dialog(int kind, bool save, bool force_in_root_dir);
	bool allow_termination();

	enum AlignType
	{
		ALIGN_LEFT,
		ALIGN_CENTER,
		ALIGN_RIGHT
	};
	void draw_str(int x, int y, const string &str, AlignType a = ALIGN_LEFT);
	void force_redraw();
	void update_menu();
	void optimize_current_view();

	ModeBase *no_mode;
	ModeBase *cur_mode;
	Array<ModeBase*> mode_queue;

	string root_dir;

	string dialog_dir[NUM_FDS], root_dir_kind[NUM_FDS];
	string dialog_file, dialog_file_complete, dialog_file_no_ending;
	Array<string> possible_sub_dir;

	MultiView::MultiView *multi_view_2d;
	MultiView::MultiView *multi_view_3d;

	Progress *progress;

	PluginManager *plugins;


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

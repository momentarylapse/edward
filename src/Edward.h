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

	virtual void onDestroy() override;

	bool handleArguments(Array<string> arg);
	void loadKeyCodes();
	void exit();

	void idleFunction();

	void onAbout();
	void onSendBugReport();
	void setMode(ModeBase *m);

	void onUpdate(Observable *o, const string &message);
	void onCommand(const string &id);
	void onClose();

	virtual void _cdecl onDrawGL();
	virtual void _cdecl onKeyDown() override;
	virtual void _cdecl onKeyUp() override;
	virtual void _cdecl onMouseMove() override;
	virtual void _cdecl onMouseWheel() override;
	virtual void _cdecl onMouseEnter() override;
	virtual void _cdecl onMouseLeave() override;
	virtual void _cdecl onLeftButtonDown() override;
	virtual void _cdecl onLeftButtonUp() override;
	virtual void _cdecl onMiddleButtonDown() override;
	virtual void _cdecl onMiddleButtonUp() override;
	virtual void _cdecl onRightButtonDown() override;
	virtual void _cdecl onRightButtonUp() override;
	void onEvent();
	void onAbortCreationMode();

	void setMessage(const string &message);
	void removeMessage();
	void errorBox(const string &message);
	Array<string> message_str;

	void onExecutePlugin();

	void setRootDirectory(const string &directory, bool compact_mode = false);
	void updateDialogDir(int kind);
	void makeDirs(const string &original_dir, bool as_root_dir = false);
	string getRootDir(int kind);

	bool fileDialog(int kind, bool save, bool force_in_root_dir);
	bool allowTermination();

	enum AlignType
	{
		ALIGN_LEFT,
		ALIGN_CENTER,
		ALIGN_RIGHT
	};
	void drawStr(int x, int y, const string &str, AlignType a = ALIGN_LEFT);
	void forceRedraw();
	void updateMenu();
	void optimizeCurrentView();

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
	virtual bool onStartup(const Array<string> &arg) override;

};

extern EdwardApp *app;

#endif /* EDWARD_H_ */

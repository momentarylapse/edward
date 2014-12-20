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

class NixTexture;
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


void read_color_4i(CFile *f, int *c);
void write_color_4i(CFile *f, int *c);
void read_color_argb(CFile *f, color &c);
void write_color_argb(CFile *f, const color &c);
void read_color_rgba(CFile *f, color &c);
void write_color_rgba(CFile *f, const color &c);
void read_color_3i(CFile *f, color &c);
void write_color_3i(CFile *f, const color &c);
color i42c(int *c);

class Edward : public Observer, public HuiWindow
{
public:
	Edward(Array<string> arg);
	virtual ~Edward();

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

	virtual void _cdecl onDraw();
	virtual void _cdecl onKeyDown();
	virtual void _cdecl onKeyUp();
	virtual void _cdecl onMouseMove();
	virtual void _cdecl onMouseWheel();
	virtual void _cdecl onMouseEnter();
	virtual void _cdecl onMouseLeave();
	virtual void _cdecl onLeftButtonDown();
	virtual void _cdecl onLeftButtonUp();
	virtual void _cdecl onMiddleButtonDown();
	virtual void _cdecl onMiddleButtonUp();
	virtual void _cdecl onRightButtonDown();
	virtual void _cdecl onRightButtonUp();
	void onEvent();
	void onAbortCreationMode();

	void setMessage(const string &message);
	void removeMessage();
	void errorBox(const string &message);
	Array<string> message_str;

	void onExecutePlugin();

	void setRootDirectory(const string &directory);
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
	bool force_redraw;
	void updateMenu();
	void optimizeCurrentView();

	ModeBase *no_mode;
	ModeBase *cur_mode;
	Array<ModeBase*> mode_queue;

	string root_dir;
	bool root_dir_correct;

	string dialog_dir[NUM_FDS], root_dir_kind[NUM_FDS];
	string dialog_file, dialog_file_complete, dialog_file_no_ending;
	Array<string> possible_sub_dir;

	MultiView::MultiView *multi_view_2d;
	MultiView::MultiView *multi_view_3d;

	Progress *progress;

	PluginManager *plugins;


	Map<NixTexture*, string> icon_image;
	string get_tex_image(NixTexture *tex);

	HuiTimer timer;
};

extern Edward *ed;

#endif /* EDWARD_H_ */

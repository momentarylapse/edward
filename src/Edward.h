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
	FDModel,
	FDTexture,
	FDSound,
	FDMaterial,
	FDTerrain,
	FDWorld,
	FDShaderFile,
	FDFont,
	FDScript,
	FDCameraFlight,
	FDFile, // any file
	NumFDs
};

// multiview data
enum{
	MVDModelSurface,
	MVDModelVertex,
	MVDModelSkinVertex,
	MVDModelPolygon,
	MVDModelEdge,
	MVDModelBall,
	MVDModelPolyhedron,
	MVDSkeletonPoint,
	MVDWorldObject,
	MVDWorldTerrain,
	MVDWorldTerrainVertex,
	MVDWorldCamPoint,
	MVDWorldCamPointVel,
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

	void onUpdate(Observable *o);
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
		AlignLeft,
		AlignCenter,
		AlignRight
	};
	void drawStr(int x, int y, const string &str, AlignType a = AlignLeft);
	void forceRedraw();
	bool force_redraw;
	void updateMenu();
	void optimizeCurrentView();

	ModeBase *cur_mode;
	Array<ModeBase*> mode_queue;

	string RootDir;
	bool RootDirCorrect;

	string DialogDir[NumFDs], RootDirKind[NumFDs];
	string DialogFile, DialogFileComplete, DialogFileNoEnding;
	Array<string> PossibleSubDir;

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

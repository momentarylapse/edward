/*
 * Edward.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef EDWARD_H_
#define EDWARD_H_

#include "lib/hui/hui.h"
#include "Mode/Mode.h"
#include "Stuff/Observer.h"
#include "Stuff/Progress.h"
#include "Stuff/PluginManager.h"



// file types
enum{
	FDModel,
	FDObject,
	FDItem,
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
	MVDModelTriangle,
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

class Edward : public Observer, public CHuiWindow
{
public:
	Edward(Array<string> arg);
	virtual ~Edward();

	bool HandleArguments(Array<string> arg);
	void LoadKeyCodes();
	int Run();

	void IdleFunction();

	void OnAbout();
	void OnSendBugReport();
	void SetMode(Mode *m);

	void OnUpdate(Observable *o);
	void OnCommand(const string &id);
	void OnClose();
	void OnDraw();

	void OnKeyDown();
	void OnKeyUp();
	void OnMouseMove();
	void OnLeftButtonDown();
	void OnLeftButtonUp();
	void OnMiddleButtonDown();
	void OnMiddleButtonUp();
	void OnRightButtonDown();
	void OnRightButtonUp();
	void OnEvent();
	void OnAbortCreationMode();

	void SetMessage(const string &message);
	void RemoveMessage();
	void ErrorBox(const string &message);
	Array<string> message_str;

	void OnExecutePlugin();

	void SetRootDirectory(const string &directory);
	void UpdateDialogDir(int kind);
	void MakeDirs(const string &original_dir, bool as_root_dir = false);

	bool FileDialog(int kind, bool save, bool force_in_root_dir);
	bool AllowTermination();

	enum AlignType
	{
		AlignLeft,
		AlignCenter,
		AlignRight
	};
	void DrawStr(int x, int y, const string &str, AlignType a = AlignLeft);
	void ForceRedraw();
	bool force_redraw;
	void UpdateMenu();

	Mode *cur_mode;

	string RootDir;
	bool RootDirCorrect;

	string DialogDir[NumFDs], RootDirKind[NumFDs];
	string DialogFile, DialogFileComplete, DialogFileNoEnding;
	Array<string> PossibleSubDir;

	MultiView *multi_view_2d;
	MultiView *multi_view_3d;

	Progress *progress;

	PluginManager *plugins;


	Array<string> icon_image;
	string get_tex_image(int tex);

	int timer;
};

extern Edward *ed;

#endif /* EDWARD_H_ */

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
#include "Mode/Welcome/ModeWelcome.h"
#include "Mode/Model/ModeModel.h"



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


void read_color_4(CFile *f,int *c);
void write_color_4(CFile *f,int *c);
color i42c(int *c);

class Edward
{
public:
	Edward(Array<string> arg);
	virtual ~Edward();

	void LoadKeyCodes();
	int Run();

	void About();
	void SetMode(Mode *m);
	void NewModel();
	bool OpenModel();
	void NewMaterial();
	bool OpenMaterial();
	void NewFont();
	bool OpenFont();
	void NewWorld();
	bool OpenWorld();

	void OnDataChange(); // TODO relocate?

	void SetMessage(const string &message);
	void ErrorBox(const string &message);


	void SetRootDirectory(const string &directory);
	void UpdateDialogDir(int kind);
	void MakeDirs(const string &original_dir, bool as_root_dir = false);

	void Draw();
	void DrawStr(int x, int y, const string &str);
	void ForceRedraw();
	bool force_redraw;

	Mode *cur_mode;

	string RootDir;
	bool RootDirCorrect;

	string DialogDir[NumFDs], RootDirKind[NumFDs];
	string DialogFile, DialogFileComplete, DialogFileNoEnding;
	Array<string> PossibleSubDir;

	CHuiWindow *win;

	MultiView *multi_view_2d;
	MultiView *multi_view_3d;
};

extern Edward *ed;

#endif /* EDWARD_H_ */

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
#include "lib/nix/nix.h"
#include "lib/script/script.h"
#ifdef _X_ALLOW_X_
#include "world/model.h"
#include "world/model_manager.h"
#include "world/material.h"
#include "world/world.h"
#include "world/camera.h"
#include "gui/gui.h"
#include "gui/font.h"
#include "fx/fx.h"
#else // for use in Edward
#include "x/material.h"
#include "x/model.h"
#include "x/model_manager.h"
#include "x/font.h"
#endif




// game configuration

EngineData Engine;


Array<XContainer*> MetaDeleteStuffList;

bool AllowXContainer = true;



// game data
string MapDir, ScriptDir;


void MetaInit()
{
	msg_db_f("Meta",1);
	Engine.DefaultFont = NULL;

	Engine.ZBufferEnabled = true;
	Engine.CullingEnabled = false;
	Engine.SortingEnabled = false;
	Engine.ConsoleEnabled = false;
	Engine.ResettingGame = false;

	Engine.FpsMax = 60;
	Engine.FpsMin = 10;

	Engine.DetailLevel = 100;
	Engine.DetailFactorInv = 1.0f;
	Engine.MirrorLevelMax = 1;
	
	Engine.TimeScale = 1.0f;
	Engine.FileErrorsAreCritical = false;

	Engine.FirstFrame = false;
	Engine.GameRunning = false;

	ModelManagerReset();
	MaterialInit();
}

void MetaEnd()
{
	MetaReset();
	MaterialEnd();
}

void MetaReset()
{
	msg_db_f("Meta reset",1);
	msg_db_m("-items",1);

	MetaDeleteStuffList.clear();

	ModelToIgnore = NULL;
	if (Gui::Fonts.num > 0)
		Engine.DefaultFont = Gui::Fonts[0];
	Engine.ShadowLight = 0;
	Engine.ShadowLowerDetail = false;
	Engine.ShadowColor = color(0.5f, 0, 0, 0);

	ModelManagerReset();
	MaterialReset();
}

void MetaSetDirs(const string &texture_dir, const string &map_dir, const string &object_dir, const string &sound_dir, const string &script_dir, const string &material_dir, const string &font_dir)
{
	NixTextureDir = texture_dir;
	NixShaderDir = material_dir;
	MapDir = map_dir;
	ObjectDir = object_dir;
	SoundDir = sound_dir;
	ScriptDir = script_dir;
	MaterialDir = material_dir;
	Gui::FontDir = font_dir;
	Script::config.Directory = script_dir;
}

void MetaCalcMove()
{
/*	for (int i=0;i<NumTextures;i++)
		NixTextureVideoMove(Texture[i],Elapsed);*/
	 msg_todo("MetaCalcMove...");
	
	Engine.DetailFactorInv = 100.0f/(float)Engine.DetailLevel;
}


void MetaDeleteLater(XContainer *p)
{
	MetaDeleteStuffList.add(p);
}

void MetaDeleteSelection()
{
	msg_db_f("MetaDeleteSelection", 1);
	for (int i=0;i<MetaDeleteStuffList.num;i++)
		delete(MetaDeleteStuffList[i]);
	MetaDeleteStuffList.clear();
}

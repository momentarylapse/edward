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

class Model;
class Object;
class Terrain;
namespace Light{
	class Light;
}

class EngineData
{
public:
	string AppName, Version;
	bool Debug, ShowTimings, ConsoleEnabled, WireMode;
	bool Record;
	float DetailLevel;
	float DetailFactorInv;
	int ShadowLevel;
	bool ShadowLowerDetail;
	Light::Light *ShadowLight;
	color ShadowColor;
	
	int Multisampling;
	bool CullingEnabled, SortingEnabled, ZBufferEnabled;
	bool ResettingGame;
	int DefaultFont;
	string InitialWorldFile, SecondWorldFile;
	bool PhysicsEnabled, CollisionsEnabled;
	int MirrorLevelMax;
	
	int NumRealColTests;
	
	float FpsMax, FpsMin;
	float TimeScale, Elapsed, ElapsedRT;

	bool FirstFrame;
	bool GameRunning;

	bool FileErrorsAreCritical;
};
extern EngineData Engine;


typedef void str_float_func(const string&,float);


void MetaInit();
void MetaEnd();
void MetaReset();
void MetaCalcMove();

// data to Meta
void MetaSetDirs(const string &texture_dir, const string &map_dir, const string &object_dir, const string &sound_dir, const string &script_dir, const string &material_dir, const string &font_dir);

// all
void _cdecl MetaDelete(void *p);
void _cdecl MetaDeleteLater(void *);
void _cdecl MetaDeleteSelection();



// game data
extern string MapDir, SoundDir, ScriptDir;

enum{
	ErrorLoadingWorld,
	ErrorLoadingMap,
	ErrorLoadingModel,
	ErrorLoadingObject,
	ErrorLoadingItem,
	ErrorLoadingScript
};

enum{
	ScriptLocationCalcMovePrae,
	ScriptLocationCalcMovePost,
	ScriptLocationRenderPrae,
	ScriptLocationRenderPost1,
	ScriptLocationRenderPost2,
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
	ScriptLocationOnKey,
	ScriptLocationOnLeftButtonDown,
	ScriptLocationOnLeftButtonUp,
	ScriptLocationOnLeftButton,
	ScriptLocationOnMiddleButtonDown,
	ScriptLocationOnMiddleButtonUp,
	ScriptLocationOnMiddleButton,
	ScriptLocationOnRightButtonDown,
	ScriptLocationOnRightButtonUp,
	ScriptLocationOnRightButton,
};



class XContainer : public VirtualBase
{
public:
	int type;
	bool enabled;

	virtual ~XContainer(){}
	virtual void _cdecl OnIterate(){}
	virtual void _cdecl OnInit(){}
	virtual void _cdecl OnDelete(){}
};

enum
{
	XContainerEffect,
	XContainerParticle,
	XContainerParticleRot,
	XContainerParticleBeam,
	XContainerGuiLayer,
	XContainerController,
	XContainerCamera,
	XContainerModel
};

extern bool AllowXContainer;

#define xcon_reg(xtype, var, array) \
	if (AllowXContainer) \
		(array).add(var); \
	(var)->type = xtype;

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


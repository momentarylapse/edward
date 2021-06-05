/*----------------------------------------------------------------------------*\
| Meta                                                                         |
| -> administration of animations, models, items, materials etc                |
| -> centralization of game data                                               |
|                                                                              |
| last updated: 2009.11.22 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#pragma once

#include "lib/base/base.h"
#include "lib/config.h"
#ifdef _X_USE_HUI_
	#include "lib/hui/hui.h"
#elif defined(_X_USE_HUI_MINIMAL_)
	#include "lib/hui_minimal/hui.h"
#endif
#include "lib/image/color.h"

class Model;
class Object;
class Terrain;
class Light;






class TemplateDataScriptVariable {
public:
	string name, value;
};

class TemplateDataScript {
public:
	Path filename;
	Array<TemplateDataScriptVariable> variables;
};



void MetaReset();
void MetaCalcMove();



enum {
	ScriptLocationCalcMovePrae,
	ScriptLocationCalcMovePost,
	ScriptLocationRenderPrae,
	ScriptLocationRenderPost,
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
	ScriptLocationOnLeftButtonDown,
	ScriptLocationOnLeftButtonUp,
	ScriptLocationOnMiddleButtonDown,
	ScriptLocationOnMiddleButtonUp,
	ScriptLocationOnRightButtonDown,
	ScriptLocationOnRightButtonUp,
};





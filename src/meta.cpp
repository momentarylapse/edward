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
#include "lib/os/msg.h"
#include "meta.h"
#ifdef _X_ALLOW_X_
#else // for use in Edward
#include "y/Font.h"
#endif
#include "y/EngineData.h"




// game configuration




void MetaReset() {
#if 0
	if (Gui::Fonts.num > 0)
		Engine.DefaultFont = Gui::Fonts[0];
#endif
	engine.shadow_lower_detail = false;
}

void MetaCalcMove() {
/*	for (int i=0;i<NumTextures;i++)
		NixTextureVideoMove(Texture[i],Elapsed);*/
	msg_todo("MetaCalcMove...");
	
	engine.detail_factor_inv = 100.0f/(float)engine.detail_level;
}



/**************************************************************************************************/
// centralization of engine data
//
// last update: 2008.06.19 (c) by MichiSoft TM
/**************************************************************************************************/

#if !defined(X_CONFIG_H__INCLUDED_)
#define X_CONFIG_H__INCLUDED_


#define _X_USE_HUI_
#define _X_USE_NET_
#define _X_USE_NIX_
#define _X_USE_IMAGE_
#define _X_USE_SCRIPT_

//#####################################################################
// Hui-API
//
// graphical user interface in the hui/* files
//#####################################################################

#define HUI_USE_GTK_ON_WINDOWS		// use gtk instead of windows api on windows



//#####################################################################
// Nix-API
//
// graphics and sound support in the nix.h and nix.cpp
//#####################################################################

#define NIX_ALLOW_API_OPENGL
//#define NIX_ALLOW_VIDEO_TEXTURE			// allow Avi-videos as texture?




//#####################################################################
// X9-Engine
//
// components each with its own file
// uncomment the ones with their files in this project
//#####################################################################


#define _X_ALLOW_CAMERA_
#define _X_ALLOW_GOD_
#define _X_ALLOW_COLLISION_
#define _X_ALLOW_PHYSICS_
#define _X_ALLOW_MATRIXN_
#define _X_ALLOW_LINKS_
#define _X_ALLOW_TREE_
#define _X_ALLOW_TERRAIN_
#define _X_ALLOW_FX_
#define _X_ALLOW_META_
#define _X_ALLOW_MODEL_
#define _X_ALLOW_OBJECT_
#define _X_ALLOW_GUI_
//#define _X_ALLOW_MODEL_ANIMATION_
#define _X_ALLOW_SCRIPT_
//#define _X_ALLOW_X_

//#define _X_DEBUG_MODEL_MANAGER_



#endif


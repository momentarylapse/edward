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




#endif


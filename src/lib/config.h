/**************************************************************************************************/
// centralization of engine data
//
// last update: 2008.06.19 (c) by MichiSoft TM
/**************************************************************************************************/

#if !defined(X_CONFIG_H__INCLUDED_)
#define X_CONFIG_H__INCLUDED_


//#define _X_USE_HUI_
#define _X_USE_XHUI_
#define _X_USE_ANY_
#define _X_USE_NET_
#define _X_USE_NIX_
#define _X_USE_IMAGE_
#define _X_USE_SCRIPT_
#define _X_USE_THREADS_

//--------------------------------------------------------------
// Hui

#define HUI_USE_GTK_ON_WINDOWS		// use gtk instead of windows api on windows

//--------------------------------------------------------------
// Image

#define IMAGE_ALLOW_PNG


//--------------------------------------------------------------
// Nix

#define NIX_ALLOW_API_OPENGL
//#define NIX_ALLOW_VIDEO_TEXTURE			// allow Avi-videos as texture?




#endif


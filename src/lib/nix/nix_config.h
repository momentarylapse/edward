/*----------------------------------------------------------------------------*\
| Nix config                                                                   |
| -> configuration for nix                                                     |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2007.11.19 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _NIX_CONFIG_EXISTS_
#define _NIX_CONFIG_EXISTS_

#include "../config.h"



// which developing environment?

#ifdef _MSC_VER
	#define NIX_IDE_VCS
	#if _MSC_VER >= 1400
		#define NIX_IDE_VCS8
	#else
		#define NIX_IDE_VCS6
	#endif
#else
	#define NIX_IDE_DEVCPP
#endif
//#define NIX_IDE_KDEVELOP ...?



// which graphics api possible?

#define NIX_API_NONE					0
#define NIX_API_OPENGL					2




#ifndef OS_WINDOWS
	#undef NIX_ALLOW_VIDEO_TEXTURE
#endif


#include <math.h>
#include "../base/base.h"
#include "../file/file.h"
#include "../hui/hui.h"
#include "../math/math.h"


typedef void callback_function();



#define NIX_MAX_TEXTURELEVELS	8

enum{
	FatalErrorNone,
	FatalErrorNoDirectX8,
	FatalErrorNoDirectX9,
	FatalErrorNoDevice,
	FatalErrorUnknownApi
};

//#define ResX	NixScreenWidth
//#define ResY	NixScreenHeight
//#define MaxX	target_width
//#define MaxY	target_height





#define AlphaNone			0
#define AlphaZero			0
#define AlphaOne			1
#define AlphaSourceColor	2
#define AlphaSourceInvColor	3
#define AlphaSourceAlpha	4
#define AlphaSourceInvAlpha	5
#define AlphaDestColor		6
#define AlphaDestInvColor	7
#define AlphaDestAlpha		8
#define AlphaDestInvAlpha	9

#define AlphaColorKey		10
#define AlphaColorKeySmooth	10
#define AlphaColorKeyHard	11
#define AlphaAdd			12
#define AlphaMaterial		13

enum{
	CULL_NONE,
	CULL_CCW,
	CULL_CW
};
#define CULL_DEFAULT		CULL_CCW

enum{
	StencilNone,
	StencilIncrease,
	StencilDecrease,
	StencilDecreaseNotNegative,
	StencilSet,
	StencilMaskEqual,
	StencilMaskNotEqual,
	StencilMaskLess,
	StencilMaskLessEqual,
	StencilMaskGreater,
	StencilMaskGreaterEqual,
	StencilReset
};

enum{
	FogLinear,
	FogExp,
	FogExp2
};

#define ShadingPlane			0
#define ShadingRound			1


namespace nix{

extern int FontHeight;
extern string FontName;

extern int Api;
extern string ApiName;
extern int ScreenWidth, ScreenHeight, ScreenDepth;		// current screen resolution
extern int DesktopWidth, DesktopHeight, DesktopDepth;	// pre-NIX-resolution
extern int target_width, target_height;						// render target size (window/texture)
extern bool Fullscreen;
extern callback_function *RefillAllVertexBuffers;			// animate the application to refill lost VertexBuffers
extern bool LightingEnabled;
extern bool CullingInverted;

extern float MouseMappingWidth, MouseMappingHeight;		// fullscreen mouse territory
extern int FatalError;
extern int NumTrias;

extern string texture_dir;
extern int TextureMaxFramesToLive, MaxVideoTextureSize;

class VertexBuffer;
extern VertexBuffer *vb_temp; // vertex buffer for 1-frame geometries
};

#endif

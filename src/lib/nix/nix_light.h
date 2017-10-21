/*----------------------------------------------------------------------------*\
| Nix light                                                                    |
| -> handling light sources                                                    |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _NIX_LIGHT_EXISTS_
#define _NIX_LIGHT_EXISTS_


namespace nix{

void _cdecl EnableLighting(bool enabled);
void _cdecl SetLightRadial(int num,const vector &pos,float radius,const color &ambient,const color &diffuse,const color &specular);
void _cdecl SetLightDirectional(int num,const vector &dir,const color &ambient,const color &diffuse,const color &specular);
void _cdecl EnableLight(int num,bool enabled);
void _cdecl SetAmbientLight(const color &c);
void _cdecl SetMaterial(const color &ambient,const color &diffuse,const color &specular,float shininess,const color &emission);
void _cdecl SpecularEnable(bool enabled);



struct Material
{
	color ambient;
	color diffusive;
	color specular;
	color emission;
	float shininess;
};
extern Material material;

struct Light
{
	color ambient;
	color diffusive;
	color specular;
	vector pos;
	float radius;
};
extern Light lights[8];

};

#endif

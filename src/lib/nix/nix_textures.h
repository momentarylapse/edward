/*----------------------------------------------------------------------------*\
| Nix textures                                                                 |
| -> texture loading and handling                                              |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2008.11.02 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _NIX_TEXTURES_EXISTS_
#define _NIX_TEXTURES_EXISTS_

namespace nix{

// textures
void init_textures();
void ReleaseTextures();
void ReincarnateTextures();
void ProgressTextureLifes();


class Texture
{
public:
	string filename;
	int width, height;
	bool is_dynamic, valid, is_cube_map;
	int life_time;
	
	unsigned int glTexture;
	unsigned int glFrameBuffer;
	unsigned int glDepthRenderBuffer;

	Image icon;

	Texture();
	~Texture();
	void _cdecl __init__();
	void _cdecl __delete__();

	void _cdecl overwrite(const Image &image);
	void _cdecl reload();
	void _cdecl unload();
	void _cdecl set_video_frame(int frame);
	void _cdecl video_move(float elapsed);
	bool _cdecl start_render();
	void _cdecl render_to_cube_map(vector &pos, callback_function *render_scene, int mask);
	void _cdecl fill_cube_map(int side, Texture *source);
};

class DynamicTexture : public Texture
{
public:
	DynamicTexture(int width, int height);
	void _cdecl __init__(int width, int height);
};

class CubeMap : public Texture
{
public:
	CubeMap(int size);
	void _cdecl __init__(int size);
};


Texture* _cdecl LoadTexture(const string &filename);
void _cdecl SetTexture(Texture *texture);
void _cdecl SetTextures(Array<Texture*> &textures);

extern Array<Texture*> textures;

extern int texture_icon_size;

};

#endif

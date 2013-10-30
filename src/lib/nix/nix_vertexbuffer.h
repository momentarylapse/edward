/*----------------------------------------------------------------------------*\
| Nix vertex buffer                                                            |
| -> handling vertex buffers                                                   |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _NIX_VERTEXBUFFER_EXISTS_
#define _NIX_VERTEXBUFFER_EXISTS_




class NixVertexBuffer
{
public:
	int numTextures;
	bool indexed;
	Array<vector> vertices;
	Array<vector> normals;
	Array<float> texCoords[NIX_MAX_TEXTURELEVELS];

	NixVertexBuffer(int num_textures);
	void _cdecl __init__(int num_textures);
	~NixVertexBuffer();
	void _cdecl __delete__();

	void _cdecl clear();
	void _cdecl draw();
	void _cdecl addTria(const vector &p1, const vector &n1, float tu1, float tv1,
							const vector &p2, const vector &n2, float tu2, float tv2,
							const vector &p3, const vector &n3, float tu3, float tv3);
	void _cdecl addTriaM(const vector &p1, const vector &n1, const float *t1,
						const vector &p2, const vector &n2, const float *t2,
						const vector &p3, const vector &n3, const float *t3);
	void _cdecl addTrias(int num_trias, const vector *p, const vector *n, const float *t);
	void _cdecl addTriasM(int num_trias, const vector *p, const vector *n, const float *t);
	void _cdecl addTriasIndexed(int num_points, int num_trias, const vector *p, const vector *n, const float *tu, const float *tv, const int *indices);
};

#define NixDraw3D(vb)  (vb)->draw()


#endif

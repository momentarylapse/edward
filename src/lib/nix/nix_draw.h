/*----------------------------------------------------------------------------*\
| Nix draw                                                                     |
| -> drawing functions                                                         |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL

#ifndef _NIX_DRAW_EXISTS_
#define _NIX_DRAW_EXISTS_

namespace nix{

class OldVertexBuffer;
class VertexBuffer;
class Texture;

void _cdecl ResetToColor(const color &c);
void _cdecl ResetZ();
void _cdecl SetColor(const color &c);
color _cdecl GetColor();
void _cdecl DrawStr(float x, float y, const string &str);
int _cdecl GetStrWidth(const string &str);
void _cdecl DrawLine(float x1, float y1, float x2, float y2, float depth);
void _cdecl DrawLine3D(const vector &l1, const vector &l2);
void _cdecl DrawLines(const Array<vector> &p, bool contiguous);
void _cdecl DrawLinesColored(const Array<vector> &p, const Array<color> &c, bool contiguous);
void _cdecl DrawRect(float x1, float x2, float y1, float y2, float depth);
void _cdecl Draw2D(const rect &src, const rect &dest, float depth);
void _cdecl Draw3D(OldVertexBuffer *vb);

void _cdecl DrawTriangles(VertexBuffer *vb);

extern float line_width;
extern bool smooth_lines;

};

#endif

#endif

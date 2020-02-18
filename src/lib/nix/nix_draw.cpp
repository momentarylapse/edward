/*----------------------------------------------------------------------------*\
| Nix draw                                                                     |
| -> drawing functions                                                         |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL

#include "nix.h"
#include "nix_common.h"

namespace nix{

float line_width = 1;
bool smooth_lines = false;
//static color current_color = White;

unsigned int line_buffer = 0;
unsigned int color_buffer = 0;


render_str_function *render_str = NULL;
extern Texture *tex_text;
extern Shader *current_shader;
extern OldVertexBuffer *vb_2d;



void SetColor(const color &c)
{
	material.emission = c;
}

color GetColor()
{
	return material.emission;
}

string str_utf8_to_ubyte(const string &str)
{
	string r;
	for (int i=0;i<str.num;i++)
		if (((unsigned int)str[i] & 0x80) > 0){
			r.add(((str[i] & 0x1f) << 6) + (str[i + 1] & 0x3f));
			i ++;
		}else
			r.add(str[i]);
	return r;
}

void DrawStr(float x, float y, const string &str)
{
	if (render_str){
		Image im;
		(*render_str)(str, im);
		if (im.width > 0){
			tex_text->overwrite(im);
			SetTexture(tex_text);
			Draw2D(rect::ID, rect(x, x + im.width, y, y + im.height), 0);
		}
	}else{
		/*string str2 = str_utf8_to_ubyte(str);

		glRasterPos3f(x, (y+2+int(float(FontHeight)*0.75f)),-1.0f);
		glListBase(OGLFontDPList);
		glCallLists(str2.num,GL_UNSIGNED_BYTE,(char*)str2.data);
		glRasterPos3f(0,0,0);
		TestGLError("DrawStr");*/
	}
}

int GetStrWidth(const string &str)
{

	if (render_str){
		Image im;
		(*render_str)(str, im);
		return im.width;
	}else{
#if 0
		string str2 = str_utf8_to_ubyte(str);
		int w = 0;
		for (int i=0;i<str2.num;i++)
			w += FontGlyphWidth[(unsigned char)str2[i]];
		return w;
#endif
	}
	return 0;
}


void DrawRect(float x1, float x2, float y1, float y2, float depth) {
	Draw2D(rect::ID, rect(x1, x2, y1, y2), depth);
}

void Draw2D(const rect &src, const rect &dest, float depth) {
	vb_2d->clear();
	vector a = vector(dest.x1, dest.y1, depth);
	vector b = vector(dest.x2, dest.y1, depth);
	vector c = vector(dest.x1, dest.y2, depth);
	vector d = vector(dest.x2, dest.y2, depth);
	vb_2d->addTria(a, v_0, src.x1, src.y1, b, v_0, src.x2, src.y1, c, v_0, src.x1, src.y2);
	vb_2d->addTria(c, v_0, src.x1, src.y2, b, v_0, src.x2, src.y1, d, v_0, src.x2, src.y2);
	Draw3D(vb_2d);
}



void Draw3D(OldVertexBuffer *vb) {
	if (vb->dirty)
		vb->update();

	current_shader->set_default_data();

	TestGLError("a");
	glEnableVertexAttribArray(0);
	TestGLError("b1");
	glBindBuffer(GL_ARRAY_BUFFER, vb->buf_v);
	TestGLError("c1");
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	TestGLError("d1");

	glEnableVertexAttribArray(1);
	TestGLError("b2");
	glBindBuffer(GL_ARRAY_BUFFER, vb->buf_n);
	TestGLError("c2");
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 0, (void*)0);
	TestGLError("d2");

	for (int i=0; i<vb->num_textures; i++) {
		glEnableVertexAttribArray(2 + i);
		TestGLError("b3");
		glBindBuffer(GL_ARRAY_BUFFER, vb->buf_t[i]);
		TestGLError("c3");
		glVertexAttribPointer(2+i, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
		TestGLError("d3");
	}

	// Draw the triangle !
	glDrawArrays(GL_TRIANGLES, 0, 3*vb->num_triangles); // Starting from vertex 0; 3 vertices total -> 1 triangle
	TestGLError("e");

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	for (int i=0; i<vb->num_textures; i++)
		glDisableVertexAttribArray(2 + i);
	TestGLError("f");


	TestGLError("Draw3D");
}

void DrawTriangles(VertexBuffer *vb) {
	if (vb->count() == 0)
		return;
	current_shader->set_default_data();

	SetVertexBuffer(vb);

	glDrawArrays(GL_TRIANGLES, 0, vb->count()); // Starting from vertex 0; 3 vertices total -> 1 triangle

	TestGLError("DrawTriangles");
}


void DrawLines(VertexBuffer *vb, bool contiguous) {
	if (vb->count() == 0)
		return;
	current_shader->set_default_data();

	SetVertexBuffer(vb);

	if (contiguous)
		glDrawArrays(GL_LINE_STRIP, 0, vb->count());
	else
		glDrawArrays(GL_LINES, 0, vb->count());
	TestGLError("DrawLines");
}


void ResetToColor(const color &c) {
	glClearColor(c.r, c.g, c.b, c.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	TestGLError("ResetToColor");
}

void ResetZ() {
	glClear(GL_DEPTH_BUFFER_BIT);
	TestGLError("ResetZ");
}

};
#endif

#include <algorithm>
#include "x.h"



extern matrix NixProjectionMatrix, NixProjectionMatrix2d, NixViewMatrix, NixWorldMatrix;

namespace Gui{

struct SubPicture
{
	Picture *picture;
	vector pos;
	color _color;
	bool enabled;
};

struct SubPicture3d
{
	Picture3d *picture_3d;
	matrix _matrix;
	float z;
	color _color;
	bool enabled;
};

struct SubText
{
	Text *text;
	vector pos;
	color _color;
	bool enabled;
};

// stuff (for the scripts)
Array<Text*> Texts;
Array<Picture*> Pictures;
Array<Picture3d*> Picture3ds;
Array<Grouping*> Groupings;
Grouping *CurrentGrouping;

void GroupingAddPicture(Grouping *grouping, Picture *picture);
void GroupingAddPicture3D(Grouping *grouping, Picture3d *picture3d);
void GroupingAddText(Grouping *grouping, Text *text);

struct Drawable
{
	float z;
	XContainer *p;

	bool operator < (const Drawable &d) const
	{	return z > d.z;	}
};

Array<Drawable> Drawables;


void Reset()
{
	msg_db_r("GuiReset",1);
	for (int i=0;i<Texts.num;i++)
		Texts[i]->used = false;
	for (int i=0;i<Pictures.num;i++)
		Pictures[i]->used = false;
	for (int i=0;i<Picture3ds.num;i++)
		Picture3ds[i]->used = false;
	for (int i=0;i<Groupings.num;i++)
		Groupings[i]->used = false;
	Drawables.clear();
	CurrentGrouping = NULL;
	msg_db_l(1);
}

inline void AddDrawable(XContainer *p, float z)
{
	Drawable d;
	d.z = z;
	d.p = p;
	Drawables.add(d);
}

void AddAllDrawables()
{
	foreach(Text *t, Texts)
		if ((t->used) && (t->enabled))
			AddDrawable(t, t->pos.z);
	foreach(Picture *p, Pictures)
		if ((p->used) && (p->enabled))
			AddDrawable(p, p->pos.z);
	foreach(Picture3d *p, Picture3ds)
		if ((p->used) && (p->enabled))
			AddDrawable(p, p->z);
}

Text *CreateText(const vector &pos, float size, const color &col, const string &str)
{
	xcont_find_new(XContainerText, Text, t, Texts);
	// default data to show existence...
	t->enabled = true;
	t->centric = false;
	t->vertical = false;
	t->pos = pos;
	t->font = Engine.DefaultFont;
	t->_color = col;
	t->size = size;
	t->text = str;
	if (CurrentGrouping)
		GroupingAddText(CurrentGrouping, t);
	return t;
}

void DeleteText(Text *text)
{
	for (int i=0;i<Texts.num;i++)
		if (Texts[i] == text){
			Texts[i]->used = false;
			break;
		}
}

Picture *CreatePicture(const vector &pos, float width, float height, int texture)
{
	xcont_find_new(XContainerPicture, Picture, p, Pictures);
	// default data to show existence...
	p->enabled = true;
	p->tc_inverted = false;
	p->source = r_id;
	p->pos = pos;
	p->width = width;
	p->height = height;
	p->_color = White;
	p->texture = texture;
	p->shader = -1;
	if (CurrentGrouping)
		GroupingAddPicture(CurrentGrouping, p);
	return p;
}

void DeletePicture(Picture *picture)
{
	for (int i=0;i<Pictures.num;i++)
		if (Pictures[i] == picture){
			Pictures[i]->used = false;
			break;
		}
}

Picture3d *CreatePicture3d(Model *model, const matrix &mat, float z)
{
	xcont_find_new(XContainerPicture3d, Picture3d, p, Picture3ds);
	// default data to show existence...
	p->enabled = true;
	p->lighting = false;
	p->world_3d = false;
	p->_color = White;
	p->z = z;
	p->model = model;
	p->_matrix = mat;
/*	else
		MatrixTranslation(p->_matrix, vector(0.5f, 0.5f, 0));*/
	if (CurrentGrouping)
		GroupingAddPicture3D(CurrentGrouping, p);
	return p;
}

void DeletePicture3d(Picture3d *picture3d)
{
	for (int i=0;i<Picture3ds.num;i++)
		if (Picture3ds[i] == picture3d){
			Picture3ds[i]->used = false;
			break;
		}
}

Grouping *CreateGrouping(const vector &pos, bool set_current)
{
	xcont_find_new(XContainerGrouping, Grouping, g, Groupings);
	// default data...
	g->enabled = true;
	g->pos = pos;
	g->_color = White;
	g->text.clear();
	g->picture.clear();
	g->picture_3d.clear();
	if (set_current)
		CurrentGrouping = g;
	return g;
}

void DeleteGrouping(Grouping *grouping)
{
	for (int i=0;i<Groupings.num;i++)
		if (Groupings[i] == grouping){
			Groupings[i]->used = false;
			break;
		}
}

void GroupingAddPicture(Grouping *grouping, Picture *picture)
{
	if ((!grouping) || (!picture))	return;
	SubPicture p;
	p.picture = picture;
	grouping->picture.add(p);
}

void GroupingAddPicture3D(Grouping *grouping, Picture3d *picture3d)
{
	if ((!grouping)||(!picture3d))	return;
	SubPicture3d p;
	p.picture_3d = picture3d;
	grouping->picture_3d.add(p);
}

void GroupingAddText(Grouping *grouping, Text *text)
{
	if ((!grouping)||(!text))	return;
	SubText t;
	t.text = text;
	grouping->text.add(t);
}

bool Picture::IsMouseOver()
{
//	if (!enabled)
//		return false;
	rect r=rect(	pos.x,	pos.x+width,
					pos.y,	pos.y+height);

	// grouping correction?
	foreachi(Grouping *g, Groupings, j){
		for (int k=0;k<g->picture.num;k++)
			if (&g->picture[k] == (void*)this){
				if (!g->enabled)
					return false;
				r.x1 += g->pos.x;
				r.x2 += g->pos.x;
				r.y1 += g->pos.y;
				r.y2 += g->pos.y;
				j+=Groupings.num;
				break;
			}
	}

	// actual test
	return ((NixMouseRel.x>r.x1)&&(NixMouseRel.x<r.x2)&&(NixMouseRel.y>r.y1)&&(NixMouseRel.y<r.y2));
}

bool Text::IsMouseOver()
{
//	if (!enabled)
//		return false;
	float w = XFGetWidth(size, text, font);
	float x = pos.x;
	if (centric)
		x-=w/2;
	rect r=rect(x,x+w,pos.y,pos.y+size);

	// grouping correction?
	foreachi(Grouping *g, Groupings, j){
		for (int k=0;k<g->text.num;k++)
			if (&g->text[k]==(void*)this){
				if (!g->enabled)
					return false;
				r.x1 += g->pos.x;
				r.x2 += g->pos.x;
				r.y1 += g->pos.y;
				r.y2 += g->pos.y;
				j+=Groupings.num;
				break;
			}
	}

	// actual test
	return ((NixMouseRel.x>r.x1)&&(NixMouseRel.x<r.x2)&&(NixMouseRel.y>r.y1)&&(NixMouseRel.y<r.y2));
}


#if 0
inline void AddToDraw(int kind,int nr,float z)
{
	int t;
	/*msg_write(string2("add %d %f",NumToDraw,z));
	for (t=0;t<NumToDraw;t++)
		msg_write(string2("%f",ToDrawZ[t]));*/
	for (t=0;t<NumToDraw;t++)
		if (z>ToDrawZ[t])
			break;
	//msg_write(string2("  -> %d",t));
	for (int n=NumToDraw;n>=t;n--){
		ToDrawKind[n+1]=ToDrawKind[n];
		ToDrawNr[n+1]=ToDrawNr[n];
		ToDrawZ[n+1]=ToDrawZ[n];
	}
	NumToDraw++;
	ToDrawKind[t]=kind;
	ToDrawNr[t]=nr;
	ToDrawZ[t]=z;
}
#endif

inline void MatrixInvH(matrix &m)
{
	m.e[13]=-m.e[13];
}

inline void MatrixAddTrans(matrix &m,vector &v)
{
	m.e[12]+=v.x;
	m.e[13]+=-v.y;
	//m.e[14]+=v.z;
}

inline void apply_grouping(Grouping *g)
{
	if (!g->used)
		return;
	for (int i=0;i<g->picture.num;i++){
		SubPicture *sp = &g->picture[i];
		Picture *p = sp->picture;
		
		sp->enabled = p->enabled;
		p->enabled &= g->enabled;
		sp->pos = p->pos;
		p->pos += g->pos;
		sp->_color = p->_color;
		p->_color = ColorMultiply(p->_color, g->_color);
	}
	for (int i=0;i<g->picture_3d.num;i++){
		SubPicture3d *sp = &g->picture_3d[i];
		Picture3d *p = sp->picture_3d;
		
		sp->enabled = p->enabled;
		p->enabled &= g->enabled;
		sp->z = p->z;
		p->z += g->pos.z;
		sp->_matrix = p->_matrix;
		MatrixAddTrans(p->_matrix, g->pos);
		sp->_color = p->_color;
		p->_color = ColorMultiply(p->_color, g->_color);
	}
	for (int i=0;i<g->text.num;i++){
		SubText *st = &g->text[i];
		Text *t = st->text;
		
		st->enabled = t->enabled;
		t->enabled &= g->enabled;
		st->pos = t->pos;
		t->pos += g->pos;
		st->_color = t->_color;
		t->_color = ColorMultiply(t->_color, g->_color);
	}
}

inline void unapply_grouping(Grouping *g)
{
	if (!g->used)
		return;
	for (int i=0;i<g->picture.num;i++){
		SubPicture *sp = &g->picture[i];
		Picture *p = sp->picture;
		
		p->enabled = sp->enabled;
		p->pos = sp->pos;
		p->_color = sp->_color;
	}
	for (int i=0;i<g->picture_3d.num;i++){
		SubPicture3d *sp = &g->picture_3d[i];
		Picture3d *p = sp->picture_3d;
		
		p->enabled = sp->enabled;
		p->z = sp->z;
		p->_matrix = sp->_matrix;
		p->_color = sp->_color;
	}
	for (int i=0;i<g->text.num;i++){
		SubText *st = &g->text[i];
		Text *t = st->text;
		
		t->enabled = st->enabled;
		t->pos = st->pos;
		t->_color = st->_color;
	}
}

inline void DrawPicture(Picture *p)
{
	if (!p->enabled)
		return;
	//if (p->_color.a <= 0)
	//	return;
#ifdef _X_ALLOW_CAMERA_
	if (Cam->shader < 0)
#endif
		NixSetShader(p->shader);
	//p->Texture=-1;

	NixSetAlpha(AlphaMaterial);
	NixSetTexture(p->texture);
	NixSetColor(p->_color);
	if (p->tc_inverted){ // texture coordinates are inverted ( y <-> x )
		// create two 3D triangles...
		vector n=v_0,pa,pb,pc,pd;
		NixVBClear(VBTemp);
		pa = p->pos;
		pb = pa+vector(p->width	,0			,0);
		pc = pa+vector(0		,p->height	,0);
		pd = pa+vector(p->width	,p->height	,0);
		NixVBAddTria(VBTemp,	pa,n,p->source.x1,p->source.y1,
								pb,n,p->source.x1,p->source.y2,
								pc,n,p->source.x2,p->source.y1);
		NixVBAddTria(VBTemp,	pc,n,p->source.x2,p->source.y1,
								pb,n,p->source.x1,p->source.y2,
								pd,n,p->source.x2,p->source.y2);
		NixDraw3D(VBTemp);
	}else{ // default
		// use the 2D rectangle drawing function
		rect d;
		d.x1= p->pos.x * MaxX;
		d.x2=(p->pos.x+p->width) * MaxX;
		d.y1= p->pos.y * MaxY;
		d.y2=(p->pos.y+p->height) * MaxY;
		NixDraw2D(p->source, d, p->pos.z);
	}
#ifdef _X_ALLOW_CAMERA_
	if (Cam->shader < 0)
#endif
		NixSetShader(-1);
}

inline void DrawText(Text *t)
{
	if (!t->enabled)
		return;
	NixSetColor(t->_color);
	if (t->vertical)
		XFDrawVertStr(t->pos.x * MaxX, t->pos.y * MaxY, t->pos.z, t->size * MaxY, t->text, t->font);
	else
		XFDrawStr(t->pos.x * MaxX, t->pos.y * MaxY, t->pos.z, t->size * MaxY, t->text, t->font, t->centric);
	//NixSetZ(true, true);
}

void mout(const matrix &m)
{
	msg_write(format("%f %f %f %f", m._00, m._01, m._02, m._03));
	msg_write(format("%f %f %f %f", m._10, m._11, m._12, m._13));
	msg_write(format("%f %f %f %f", m._20, m._21, m._22, m._23));
	msg_write(format("%f %f %f %f", m._30, m._31, m._32, m._33));
}


inline void DrawPicture3d(Picture3d *p)
{
	if (!p->enabled)
		return;
	if (!p->model)
		return;
#ifdef _X_ALLOW_MODEL_
	bool ch_alpha = (p->_color.a < 1);
	bool ch_color = ((p->_color.a < 1) || (p->_color.r < 1) || (p->_color.g < 1) || (p->_color.b < 1));
	//CModel *m = p->model;
	Material *m = &p->model->material[0];
	if (ch_color){
		p->ambient=m->ambient;
		p->diffuse=m->diffuse;
		p->emission=m->emission;
		m->ambient=ColorMultiply(m->ambient,p->_color);
		m->diffuse=ColorMultiply(m->diffuse,p->_color);
		m->emission=ColorMultiply(m->emission,p->_color);
		if (ch_alpha){
			if (m->transparency_mode != TransparencyModeFactor)
				m->alpha_factor = 1;
			p->transparency_mode = m->transparency_mode;
			m->transparency_mode = TransparencyModeFactor;
			p->alpha_factor = m->alpha_factor;
			m->alpha_factor *= p->_color.a;
			p->reflection_density = m->reflection_density;
			m->reflection_density *= p->_color.a;
		}
	}


	if ((ch_alpha)||(m->transparency_mode>0))
		NixSetZ(false,true);
	else
		NixSetZ(true,true);
	if (p->world_3d){
		NixSetZ(false,false);
	}
	NixSetAlpha(AlphaNone);
	NixEnableLighting(p->lighting);
	if (p->world_3d){
		NixSetProjection(true, true);
		cur_cam->SetView();
		p->model->_matrix = p->_matrix;
	}else{
		NixSetProjection(false, true);
		matrix s, t;
		//MatrixInvH(p->_matrix);
		/*if (p->relative)
			p->model->_matrix = t * rel * p->_matrix;
		else
			p->model->_matrix = t * p->_matrix;*/
		MatrixTranslation(t, e_y);
		MatrixScale(s, 1, -1, 1);
		NixSetColor(White);
		matrix projection = NixProjectionMatrix * t * s;
		NixSetProjectionMatrix(projection);
		p->model->_matrix = p->_matrix;
	}

	// draw
//	p->model->Draw(SkinView0,&p->_matrix,false);
	p->model->Draw(SkinHigh,false,false);
	//MatrixInvH(p->_matrix);

	// clean up
	/*if (p->world_3d){
		NixSetProjection(false, true);
		NixSetView(m_id);
	}
	NixSetWorldMatrix(m_id);*/
	NixSetProjection(false, true);
	NixEnableLighting(false);
	NixSetZ(false, true);

	if (ch_color){
		m->ambient = p->ambient;
		m->diffuse = p->diffuse;
		m->emission = p->emission;
		if (ch_alpha){
			m->transparency_mode = p->transparency_mode;
			m->alpha_factor = p->alpha_factor;
			m->reflection_density = p->reflection_density;
		}
	}
#endif
}

void Draw()
{
	msg_db_r("GuiDraw", 2);

	NixEnableLighting(false);


	// groupings
	for (int i=0;i<Groupings.num;i++)
		apply_grouping(Groupings[i]);

	// sorting
	AddAllDrawables();
	std::sort(&Drawables[0], &Drawables[Drawables.num]);

	// drawing
	NixSetAlpha(AlphaNone);
	//NixSetProjection(false, true);
	NixSetView(m_id);
#ifdef _X_ALLOW_CAMERA_
	if (Cam->shaded_displays)
		NixSetShader(Cam->shader);
#endif

	//NixSetZ(true, true);
	NixSetZ(false, true);
	NixEnableLighting(false);
	NixSetProjection(false, true);
	NixSpecularEnable(false);

	 // drawing
	foreach(Drawable &d, Drawables){
		int type = d.p->type;
		if (type == XContainerText)
			DrawText((Text*)d.p);
		else if (type == XContainerPicture)
			DrawPicture((Picture*)d.p);
		else if (type == XContainerPicture3d)
			DrawPicture3d((Picture3d*)d.p);
		/*else if (type == XContainerView){
		}*/
	}
	Drawables.clear();
	NixSetAlpha(AlphaNone);
	NixSetZ(true, true);
	NixSpecularEnable(true);
	NixSetShader(-1);

	// groupings
	for (int i=0;i<Groupings.num;i++)
		unapply_grouping(Groupings[i]);
	 msg_db_l(2);
}

};

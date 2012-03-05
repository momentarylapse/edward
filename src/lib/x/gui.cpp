#include <algorithm>
#include "x.h"


// stuff (for the scripts)
Array<sText*> Text;
Array<sPicture*> Picture;
Array<sPicture3D*> Picture3D;
Array<sGrouping*> Grouping;
sGrouping *CurrentGrouping;

void GuiGroupingAddPicture(sGrouping *grouping, sPicture *picture);
void GuiGroupingAddPicture3D(sGrouping *grouping, sPicture3D *picture3d);
void GuiGroupingAddText(sGrouping *grouping, sText *text);

struct sDrawable
{
	float z;
	XContainer *p;

	bool operator < (const sDrawable &d) const
	{	return z > d.z;	}
};

Array<sDrawable> Drawable;


void GuiReset()
{
	msg_db_r("GuiReset",1);
	for (int i=0;i<Text.num;i++)
		Text[i]->used = false;
	for (int i=0;i<Picture.num;i++)
		Picture[i]->used = false;
	for (int i=0;i<Picture3D.num;i++)
		Picture3D[i]->used = false;
	for (int i=0;i<Grouping.num;i++)
		Grouping[i]->used = false;
	Drawable.clear();
	CurrentGrouping = NULL;
	msg_db_l(1);
}

inline void AddDrawable(XContainer *p, float z)
{
	sDrawable d;
	d.z = z;
	d.p = p;
	Drawable.add(d);
}

void AddAllDrawables()
{
	foreach(Text, t)
		if ((t->used) && (t->enabled))
			AddDrawable(t, t->pos.z);
	foreach(Picture, p)
		if ((p->used) && (p->enabled))
			AddDrawable(p, p->pos.z);
	foreach(Picture3D, p)
		if ((p->used) && (p->enabled))
			AddDrawable(p, p->z);
}

sText *GuiCreateText(const vector &pos, float size, const color &col, const string &str)
{
	xcont_find_new(XContainerText, sText, t, Text);
	// default data to show existence...
	t->enabled = true;
	t->centric = false;
	t->vertical = false;
	t->pos = pos;
	t->font = DefaultFont;
	t->_color = col;
	t->size = size;
	t->text = str;
	if (CurrentGrouping)
		GuiGroupingAddText(CurrentGrouping, t);
	return t;
}

void GuiDeleteText(sText *text)
{
	for (int i=0;i<Text.num;i++)
		if (Text[i] == text){
			Text[i]->used = false;
			break;
		}
}

sPicture *GuiCreatePicture(const vector &pos, float width, float height, int texture)
{
	xcont_find_new(XContainerPicture, sPicture, p, Picture);
	// default data to show existence...
	p->enabled = true;
	p->tc_inverted = false;
	p->source = r01;
	p->pos = pos;
	p->width = width;
	p->height = height;
	p->_color = White;
	p->texture = texture;
	p->shader = -1;
	if (CurrentGrouping)
		GuiGroupingAddPicture(CurrentGrouping, p);
	return p;
}

void GuiDeletePicture(sPicture *picture)
{
	for (int i=0;i<Picture.num;i++)
		if (Picture[i] == picture){
			Picture[i]->used = false;
			break;
		}
}

sPicture3D *GuiCreatePicture3D(CModel *model, const matrix &mat, float z)
{
	xcont_find_new(XContainerPicture3d, sPicture3D, p, Picture3D);
	// default data to show existence...
	p->enabled = true;
	p->relative = true;
	p->lighting = false;
	p->world_3d = false;
	p->_color = White;
	p->z = z;
	p->model = model;
	p->_matrix = mat;
/*	else
		MatrixTranslation(p->_matrix, vector(0.5f, 0.5f, 0));*/
	if (CurrentGrouping)
		GuiGroupingAddPicture3D(CurrentGrouping, p);
	return p;
}

void GuiDeletePicture3D(sPicture3D *picture3d)
{
	for (int i=0;i<Picture3D.num;i++)
		if (Picture3D[i] == picture3d){
			Picture3D[i]->used = false;
			break;
		}
}

sGrouping *GuiCreateGrouping(const vector &pos, bool set_current)
{
	xcont_find_new(XContainerGrouping, sGrouping, g, Grouping);
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

void GuiDeleteGrouping(sGrouping *grouping)
{
	for (int i=0;i<Grouping.num;i++)
		if (Grouping[i] == grouping){
			Grouping[i]->used = false;
			break;
		}
}

void GuiGroupingAddPicture(sGrouping *grouping, sPicture *picture)
{
	if ((!grouping) || (!picture))	return;
	sSubPicture p;
	p.picture = picture;
	grouping->picture.add(p);
}

void GuiGroupingAddPicture3D(sGrouping *grouping, sPicture3D *picture3d)
{
	if ((!grouping)||(!picture3d))	return;
	sSubPicture3D p;
	p.picture_3d = picture3d;
	grouping->picture_3d.add(p);
}

void GuiGroupingAddText(sGrouping *grouping, sText *text)
{
	if ((!grouping)||(!text))	return;
	sSubText t;
	t.text = text;
	grouping->text.add(t);
}

bool GuiMouseOver(void *p)
{
	// test pictures
	for (int i=0;i<Picture.num;i++)
		if (Picture[i] == p){
		//	if (!Picture[i].Enabled)
		//		return false;
			rect r=rect(	Picture[i]->pos.x,	Picture[i]->pos.x+Picture[i]->width,
							Picture[i]->pos.y,	Picture[i]->pos.y+Picture[i]->height);

			// grouping correction?
			for (int j=0;j<Grouping.num;j++){
				sGrouping *g = Grouping[j];
				for (int k=0;k<g->picture.num;k++)
					if (&g->picture[k] == p){
						if (!g->enabled)
							return false;
						r.x1 += g->pos.x;
						r.x2 += g->pos.x;
						r.y1 += g->pos.y;
						r.y2 += g->pos.y;
						j+=Grouping.num;
						break;
					}
			}

			// actual test
			return ((NixMouseRel.x>r.x1)&&(NixMouseRel.x<r.x2)&&(NixMouseRel.y>r.y1)&&(NixMouseRel.y<r.y2));
		}

	// test texts
	for (int i=0;i<Text.num;i++)
		if (Text[i] == p){
	//		if (!Text[i].Enabled)
	//			return false;
			XFontIndex = Text[i]->font;
			float w = XFGetWidth(Text[i]->size, Text[i]->text);
			float x = Text[i]->pos.x;
			if (Text[i]->centric)
				x-=w/2;
			rect r=rect(x,x+w,Text[i]->pos.y,Text[i]->pos.y+Text[i]->size);

			// grouping correction?
			for (int j=0;j<Grouping.num;j++){
				sGrouping *g = Grouping[j];
				for (int k=0;k<g->text.num;k++)
					if (&g->text[k]==p){
						if (!g->enabled)
							return false;
						r.x1 += g->pos.x;
						r.x2 += g->pos.x;
						r.y1 += g->pos.y;
						r.y2 += g->pos.y;
						j+=Grouping.num;
						break;
					}
			}

			// actual test

			return ((NixMouseRel.x>r.x1)&&(NixMouseRel.x<r.x2)&&(NixMouseRel.y>r.y1)&&(NixMouseRel.y<r.y2));
		}
	return false;
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

inline void MatrixAddTrans(matrix &m,vector &v,bool rel)
{
	if (rel){
		m.e[12]+=v.x;
		m.e[13]+=-v.y;
		//m.e[14]+=v.z;
	}else{
		m.e[12]+=v.x*(float)MaxX;
		m.e[13]+=-v.y*(float)MaxY;
		//m.e[14]+=v.z;
	}
}

inline void apply_grouping(sGrouping *g)
{
	if (!g->used)
		return;
	for (int i=0;i<g->picture.num;i++){
		sSubPicture *sp = &g->picture[i];
		sPicture *p = sp->picture;
		
		sp->enabled = p->enabled;
		p->enabled &= g->enabled;
		sp->pos = p->pos;
		p->pos += g->pos;
		sp->_color = p->_color;
		p->_color = ColorMultiply(p->_color, g->_color);
	}
	for (int i=0;i<g->picture_3d.num;i++){
		sSubPicture3D *sp = &g->picture_3d[i];
		sPicture3D *p = sp->picture_3d;
		
		sp->enabled = p->enabled;
		p->enabled &= g->enabled;
		sp->z = p->z;
		p->z += g->pos.z;
		sp->_matrix = p->_matrix;
		MatrixAddTrans(p->_matrix, g->pos, p->relative);
		sp->_color = p->_color;
		p->_color = ColorMultiply(p->_color, g->_color);
	}
	for (int i=0;i<g->text.num;i++){
		sSubText *st = &g->text[i];
		sText *t = st->text;
		
		st->enabled = t->enabled;
		t->enabled &= g->enabled;
		st->pos = t->pos;
		t->pos += g->pos;
		st->_color = t->_color;
		t->_color = ColorMultiply(t->_color, g->_color);
	}
}

inline void unapply_grouping(sGrouping *g)
{
	if (!g->used)
		return;
	for (int i=0;i<g->picture.num;i++){
		sSubPicture *sp = &g->picture[i];
		sPicture *p = sp->picture;
		
		p->enabled = sp->enabled;
		p->pos = sp->pos;
		p->_color = sp->_color;
	}
	for (int i=0;i<g->picture_3d.num;i++){
		sSubPicture3D *sp = &g->picture_3d[i];
		sPicture3D *p = sp->picture_3d;
		
		p->enabled = sp->enabled;
		p->z = sp->z;
		p->_matrix = sp->_matrix;
		p->_color = sp->_color;
	}
	for (int i=0;i<g->text.num;i++){
		sSubText *st = &g->text[i];
		sText *t = st->text;
		
		t->enabled = st->enabled;
		t->pos = st->pos;
		t->_color = st->_color;
	}
}

inline void GuiDrawPicture(sPicture *p)
{
	if (!p->enabled)
		return;
#ifdef _X_ALLOW_CAMERA_
	if (Cam->shader < 0)
#endif
		NixSetShader(p->shader);
	//p->Texture=-1;
	NixSetZ(false, true);
	NixSetAlpha(AlphaMaterial);
	NixSetCull(CullNone);
	if (p->tc_inverted){ // texture coordinates are inverted ( y <-> x )
		// create two 3D triangles...
		vector n=v0,pa,pb,pc,pd;
		NixVBClear(VBTemp);
		pa=   vector((p->pos.x-0.5f)*float(MaxX)	,(0.5f-p->pos.y)*float(MaxY),0);
		pb=pa+vector(p->width*float(MaxX)			,0							,0);
		pc=pa+vector(0								,-p->height*float(MaxY)		,0);
		pd=pa+vector(p->width*float(MaxX)			,-p->height*float(MaxY)		,0);
		NixVBAddTria(VBTemp,	pa,n,p->source.x1,p->source.y1,
								pb,n,p->source.x1,p->source.y2,
								pc,n,p->source.x2,p->source.y1);
		NixVBAddTria(VBTemp,	pc,n,p->source.x2,p->source.y1,
								pb,n,p->source.x1,p->source.y2,
								pd,n,p->source.x2,p->source.y2);
		NixSetMaterial(Black,p->_color,Black,0,p->_color);
		NixEnableLighting(true);
		NixDraw3D(p->texture,VBTemp,m_id);
	}else{ // default
		// use the 2D rectangle drawing function
		rect d;
		d.x1= p->pos.x				*float(MaxX);
		d.x2=(p->pos.x+p->width)	*float(MaxX);
		d.y1= p->pos.y				*float(MaxY);
		d.y2=(p->pos.y+p->height)	*float(MaxY);
		NixDraw2D(p->texture, p->_color, p->source, d, p->pos.z);
	}
#ifdef _X_ALLOW_CAMERA_
	if (Cam->shader < 0)
#endif
		NixSetShader(-1);
}

inline void GuiDrawText(sText *t)
{
	if (!t->enabled)
		return;
	//NixSetAlpha(AlphaNone);
	XFontColor = t->_color;
	XFontIndex = t->font;
	XFontZ = t->pos.z;
	NixSetZ(false, true);
	NixSetAlpha(AlphaMaterial);
	NixSetCull(CullNone);
	if (t->vertical)
		XFDrawVertStr(t->pos.x, t->pos.y, t->size, t->text);
	else
		XFDrawStr(t->pos.x, t->pos.y, t->size, t->text, t->centric);
	//NixSetZ(true, true);
}


static matrix rel;

inline void GuiDrawPicture3D(sPicture3D *p)
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
		view_cur->SetView();
		NixSetZ(false,false);
	}
	NixSetAlpha(AlphaNone);
	NixSetCull(CullDefault);
	NixEnableLighting(p->lighting);
	if (p->world_3d){
		p->model->_matrix = p->_matrix;
	}else{
		matrix t;
		MatrixTranslation(t,vector(-(float)MaxX/2,-(float)MaxY/2,p->z*100));
		//MatrixInvH(p->_matrix);
		if (p->relative){
			MatrixMultiply(p->model->_matrix,rel,p->_matrix);
			MatrixMultiply(p->model->_matrix,t,p->model->_matrix);
		}else
			MatrixMultiply(p->model->_matrix,t,p->_matrix);
	}
//	p->model->Draw(SkinView0,&p->_matrix,false);
	p->model->Draw(SkinHigh,false,false);
	//MatrixInvH(p->_matrix);
	
	if (p->world_3d)
		NixSetView(false, v0, v0, vector(1,1,0.1f));

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

void GuiDraw()
{
	msg_db_r("GuiDraw", 2);
	// save old state
	int _XFontIndex = XFontIndex;
	color _XFontColor = XFontColor;
	float _XFontZ = XFontZ;

	//NixSetView2D();
	//NixSetView(false);
	NixSetCull(CullNone);
	NixEnableLighting(false);
	// reset Z buffer
//	NixSetZ(true,false);
//	NixSetAlphaSD(AlphaZero,AlphaOne);
//	NixDraw2D(-1,NULL,NULL,NULL,0.9999999f);


	// groupings
	for (int i=0;i<Grouping.num;i++)
		apply_grouping(Grouping[i]);

	// sorting
	AddAllDrawables();
	std::sort(&Drawable[0], &Drawable[Drawable.num]);

	MatrixScale(rel, float(MaxX), float(MaxY), 1.0f);

	// drawing
	NixSetAlpha(AlphaNone);
	NixSetView(false, v0, v0, vector(1,1,0.1f));
#ifdef _X_ALLOW_CAMERA_
	if (Cam->shaded_displays)
		NixSetShader(Cam->shader);
#endif

	NixSetZ(true, true);

	 // drawing
	foreach(Drawable, d){
		int type = d.p->type;
		if (type == XContainerText)
			GuiDrawText((sText*)d.p);
		else if (type == XContainerPicture)
			GuiDrawPicture((sPicture*)d.p);
		else if (type == XContainerPicture3d)
			GuiDrawPicture3D((sPicture3D*)d.p);
		/*else if (type == XContainerView){
		}*/
	}
	Drawable.clear();
	NixSetAlpha(AlphaNone);
	NixSetZ(true, true);
	NixSetCull(CullDefault);
	NixSetShader(-1);
	XFontColor = _XFontColor;
	XFontIndex = _XFontIndex;
	XFontZ =_XFontZ;

	// groupings
	for (int i=0;i<Grouping.num;i++)
		unapply_grouping(Grouping[i]);
	 msg_db_l(2);
}

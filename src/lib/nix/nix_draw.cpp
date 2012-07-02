/*----------------------------------------------------------------------------*\
| Nix draw                                                                     |
| -> drawing functions                                                         |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#include "nix.h"
#include "nix_common.h"

static color FontColor = White;
float NixLineWidth = 1;
bool NixSmoothLines = false;


void NixSetFontColor(const color &c)
{
	FontColor=c;
}

void NixDrawChar(int x,int y,char c)
{
	char str[2];
	str[0]=c;
	str[1]=0;
	NixDrawStr(x,y,str);
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

void NixDrawStr(int x,int y,const string &str)
{
	msg_db_r("NixDrawStr",10);
	string str2 = str_utf8_to_ubyte(str);

	NixSetTexture(-1);
	OGLSet2DMode();
	bool z_test, z_write;
	glGetBooleanv(GL_DEPTH_TEST, (GLboolean*)&z_test);
	glGetBooleanv(GL_DEPTH_WRITEMASK, (GLboolean*)&z_write);
	if (z_test)
		NixSetZ(false, false);
	glColor3f(FontColor.r,FontColor.g,FontColor.b);
	glRasterPos3f(float(x),float(y+2+int(float(NixFontHeight)*0.75f)),-1.0f);
	glListBase(NixOGLFontDPList);
	glCallLists(str2.num,GL_UNSIGNED_BYTE,(char*)str2.data);
	glRasterPos3f(0,0,0);
	if (z_test)
		NixSetZ(z_write, z_test);
	msg_db_l(10);
}

int NixGetStrWidth(const string &str)
{
	string str2 = str_utf8_to_ubyte(str);
	int w = 0;
	for (int i=0;i<str2.num;i++)
		w += NixFontGlyphWidth[(unsigned char)str2[i]];
	return w;
}

void NixDrawFloat(int x,int y,float fl,int com)
{
	NixDrawStr(x,y,f2s(fl,com));
}

void NixDrawInt(int x,int y,int num)
{
	NixDrawStr(x,y,i2s(num));
}

void NixDrawLine(float x1,float y1,float x2,float y2,const color &c,float depth)
{
	float dx=x2-x1;
	if (dx<0)	dx=-dx;
	float dy=y2-y1;
	if (dy<0)	dy=-dy;
	OGLSet2DMode();
	glDisable(GL_TEXTURE_2D);
	glColor3fv((float*)&c);

#ifdef NIX_OS_LINUX
	// internal line drawing function \(^_^)/
	if (NixSmoothLines){
		// antialiasing!
		glLineWidth(NixLineWidth + 0.5);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}else
		glLineWidth(NixLineWidth);
		glBegin(GL_LINES);
			glVertex3f((float)x1,(float)y1,depth);
			glVertex3f((float)x2,(float)y2,depth);
		glEnd();
	if (NixSmoothLines){
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_BLEND);
	}
#else

	// own line drawing function (T_T)
	if (dx>dy){
		if (x1>x2){
			float x=x2;	x2=x1;	x1=x;
			float y=y2;	y2=y1;	y1=y;
		}
		glBegin(GL_TRIANGLES);
			glVertex3f(x1,y1+1,depth);
			glVertex3f(x1,y1  ,depth);
			glVertex3f(x2,y2+1,depth);
			glVertex3f(x2,y2  ,depth);
			glVertex3f(x2,y2+1,depth);
			glVertex3f(x1,y1  ,depth);
		glEnd();
	}else{
		if (y1<y2){
			float x=x2;	x2=x1;	x1=x;
			float y=y2;	y2=y1;	y1=y;
		}
		glBegin(GL_TRIANGLES);
			glVertex3f(x1+1,y1,depth);
			glVertex3f(x1  ,y1,depth);
			glVertex3f(x2+1,y2,depth);
			glVertex3f(x2  ,y2,depth);
			glVertex3f(x2+1,y2,depth);
			glVertex3f(x1  ,y1,depth);
		glEnd();
	}
#endif
}

void NixDrawLines(float *x, float *y, int num_lines, bool contiguous, const color &c, float depth)
{
	OGLSet2DMode();
	glDisable(GL_TEXTURE_2D);
	glColor3fv((float*)&c);

	// internal line drawing function \(^_^)/
	if (NixSmoothLines){
		// antialiasing!
		glLineWidth(NixLineWidth + 0.5);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}else
		glLineWidth(NixLineWidth);

		if (contiguous){
			glBegin(GL_LINE_STRIP);
				glVertex3f(*x, *y, depth);	x++;	y++;
				for (int i=0;i<num_lines;i++){
					glVertex3f(*x, *y, depth);	x++;	y++;
				}
			glEnd();
		}else{
			glBegin(GL_LINES);
				for (int i=0;i<num_lines;i++){
					glVertex3f(*x, *y, depth);	x++;	y++;
					glVertex3f(*x, *y, depth);	x++;	y++;
				}
			glEnd();
		}
	if (NixSmoothLines){
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_BLEND);
	}
}

void NixDrawLineV(int x,int y1,int y2,const color &c,float depth)
{
	/*NixDrawLine((float)x,(float)y1,(float)x,(float)y2,c,depth);
	return;*/
	if (y1>y2){
		int y=y2;	y2=y1;	y1=y;
	}
	rect d;
	d.x1=(float)x;
	d.x2=(float)x+1;
	d.y1=(float)y1;
	d.y2=(float)y2;
	NixDraw2D(-1, c, r_id, d, depth);
}

void NixDrawLineH(int x1,int x2,int y,const color &c,float depth)
{
	/*NixDrawLine((float)x1,(float)y,(float)x2,(float)y,c,depth);
	return;*/
	if (x1>x2){
		int x=x2;
		x2=x1;
		x1=x;
	}
	rect d;
	d.x1=(float)x1;
	d.x2=(float)x2;
	d.y1=(float)y;
	d.y2=(float)y+1;
	NixDraw2D(-1, c, r_id, d, depth);
}

void NixDrawLine3D(const vector &l1,const vector &l2,const color &c)
{
/*	OGLSet2DMode();
	glDisable(GL_TEXTURE_2D);
	glColor3fv(color2f3(c));
	glLineWidth(1);
	glBegin(GL_LINE);
		glVertex3fv((float*)&l1);
		glVertex3fv((float*)&l2);
	glEnd();*/
		vector p1, p2;
		NixGetVecProject(p1,l1);
		NixGetVecProject(p2,l2);
		if ((p1.z>0)&&(p2.z>0)&&(p1.z<1)&&(p2.z<1))
			NixDrawLine(p1.x,p1.y,p2.x,p2.y,c,(p1.z+p2.z)/2);
}

void NixDrawRect(float x1,float x2,float y1,float y2,const color &c,float depth)
{
	float t;
	if (x1>x2){
		t=x1;	x1=x2;	x2=t;
	}
	if (y1>y2){
		t=y1;	y1=y2;	y2=t;
	}
	if (!NixFullscreen){
		int pa=40;
		for (int i=0;i<int(x2-x1-1)/pa+1;i++){
			for (int j=0;j<int(y2-y1-1)/pa+1;j++){
				float _x1=x1+i*pa;
				float _y1=y1+j*pa;

				float _x2=x2;
				if (x2-x1-i*pa>pa)	_x2=x1+i*pa+pa;
				float _y2=y2;
				if (y2-y1-j*pa>pa)	_y2=y1+j*pa+pa;

				rect r=rect(_x1,_x2,_y1,_y2);
				NixDraw2D(-1,c,NixTargetRect,r,depth);
			}
		}
		return;
	}
	rect r=rect(x1,x2,y1,y2);
	NixDraw2D(-1,c,NixTargetRect,r,depth);
}

inline void SetShaderFileData(int texture0,int texture1,int texture2,int texture3)
{
#ifdef NIX_OS_LINUX
		if (NixglShaderCurrent==0)	return;
		int loc;
		loc=glGetUniformLocationARB(NixglShaderCurrent,"tex0");
		if (loc>-1)	glUniform1iARB(loc, 0);
		loc=glGetUniformLocationARB(NixglShaderCurrent,"tex1");
		if (loc>-1)	glUniform1iARB(loc, 1);
		loc=glGetUniformLocationARB(NixglShaderCurrent,"tex2");
		if (loc>-1)	glUniform1iARB(loc, 2);
		loc=glGetUniformLocationARB(NixglShaderCurrent,"tex3");
		if (loc>-1)	glUniform1iARB(loc, 3);
#else
	msg_todo("SetShaderFileData windows?");
#endif
}

void NixDraw2D(int texture,const color &col,const rect &src,const rect &dest,float depth)
{
	//if (depth==0)	depth=0.5f;
	
	//msg_write("2D");
	NixSetTexture(texture);
	SetShaderFileData(texture,-1,-1,-1);
	depth=depth*2-1;
	OGLSet2DMode();
	glColor4fv((float*)&col);
	glBegin(GL_QUADS);
		glTexCoord2f(src.x1,1-src.y2);
		glVertex3f(dest.x1,dest.y2,depth);
		glTexCoord2f(src.x1,1-src.y1);
		glVertex3f(dest.x1,dest.y1,depth);
		glTexCoord2f(src.x2,1-src.y1);
		glVertex3f(dest.x2,dest.y1,depth);
		glTexCoord2f(src.x2,1-src.y2);
		glVertex3f(dest.x2,dest.y2,depth);
	glEnd();
}

void NixDraw3D(int texture,int buffer,const matrix &mat)
{
	if (buffer<0)	return;

	sVertexBuffer *vb = &NixVB[buffer];
	
	NixSetTexture(texture);
	// Transformations-Matrix Modell->Welt
	NixSetWorldMatrix(mat);

	SetShaderFileData(texture,-1,-1,-1);

	OGLSet3DMode();
#if 1
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_NORMAL_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	glVertexPointer( 3, GL_FLOAT, 0, vb->glVertices );
	glNormalPointer( GL_FLOAT, 0, vb->glNormals );
#ifdef NIX_OS_LINUX
	glClientActiveTexture(GL_TEXTURE0_ARB);
#endif
	glTexCoordPointer( 2, GL_FLOAT, 0, vb->glTexCoords[0] );
	glDrawArrays(GL_TRIANGLES,0,vb->NumTrias*3);
	//glDrawArrays(GL_TRIANGLE_STRIP,0,vb->NumTrias*3);
#else

	for (int i=0;i<VBNumTrias[buffer];i++){
		glBegin(GL_TRIANGLES);
			glTexCoord2f(	vb->glVertices[i*3  ].tu,vb->glVertices[i*3  ].tv);
			glNormal3f(		vb->glVertices[i*3  ].nx,vb->glVertices[i*3  ].ny,vb->glVertices[i*3  ].nz);
			glVertex3f(		vb->glVertices[i*3  ].x ,vb->glVertices[i*3  ].y ,vb->glVertices[i*3  ].z );
			glTexCoord2f(	vb->glVertices[i*3+1].tu,vb->glVertices[i*3+1].tv);
			glNormal3f(		vb->glVertices[i*3+1].nx,vb->glVertices[i*3+1].ny,vb->glVertices[i*3+1].nz);
			glVertex3f(		vb->glVertices[i*3+1].x ,vb->glVertices[i*3+1].y ,vb->glVertices[i*3+1].z );
			glTexCoord2f(	vb->glVertices[i*3+2].tu,vb->glVertices[i*3+2].tv);
			glNormal3f(		vb->glVertices[i*3+2].nx,vb->glVertices[i*3+2].ny,vb->glVertices[i*3+2].nz);
			glVertex3f(		vb->glVertices[i*3+2].x ,vb->glVertices[i*3+2].y ,vb->glVertices[i*3+2].z );
		glEnd();
	}
#endif
	NixNumTrias += vb->NumTrias;
}

void NixDraw3DM(int *texture,int buffer,const matrix &mat)
{
	if (buffer<0)	return;
	sVertexBuffer *vb = &NixVB[buffer];
	NixSetTextures(texture,vb->NumTextures);
	// Transformations-Matrix Modell->Welt
	NixSetWorldMatrix(mat);

	SetShaderFileData(texture[0],texture[1],texture[2],texture[3]);

	OGLSet3DMode();
#if 1
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_NORMAL_ARRAY );
	glVertexPointer( 3, GL_FLOAT, 0, vb->glVertices );
	glNormalPointer( GL_FLOAT, 0, vb->glNormals );

	// set multitexturing
	if (OGLMultiTexturingSupport){
		for (int i=0;i<vb->NumTextures;i++){
			glActiveTexture(GL_TEXTURE0+i);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,NixTexture[texture[i]].glTexture);
			glClientActiveTexture(GL_TEXTURE0+i);
			glEnableClientState( GL_TEXTURE_COORD_ARRAY );
			glTexCoordPointer( 2, GL_FLOAT, 0, vb->glTexCoords[i] );
		}
	}else{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,NixTexture[texture[0]].glTexture);
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );
		glTexCoordPointer( 2, GL_FLOAT, 0, vb->glTexCoords[0] );
	}

	// draw
	glDrawArrays(GL_TRIANGLES,0,vb->NumTrias*3);

	// unset multitexturing
	if (OGLMultiTexturingSupport){
		for (int i=1;i<vb->NumTextures;i++){
			glActiveTexture(GL_TEXTURE0+i);
			glDisable(GL_TEXTURE_2D);
			glClientActiveTexture(GL_TEXTURE0+i);
			glDisableClientState( GL_TEXTURE_COORD_ARRAY );
		}
		glActiveTexture(GL_TEXTURE0);
		glClientActiveTexture(GL_TEXTURE0);
	}
#else

	for (int i=0;i<vb->NumTrias;i++){
		glBegin(GL_TRIANGLES);
			glTexCoord2f(	vb->glVertices[i*3  ].tu,vb->glVertices[i*3  ].tv);
			glNormal3f(		vb->glVertices[i*3  ].nx,vb->glVertices[i*3  ].ny,vb->glVertices[i*3  ].nz);
			glVertex3f(		vb->glVertices[i*3  ].x ,vb->glVertices[i*3  ].y ,vb->glVertices[i*3  ].z );
			glTexCoord2f(	vb->glVertices[i*3+1].tu,vb->glVertices[i*3+1].tv);
			glNormal3f(		vb->glVertices[i*3+1].nx,vb->glVertices[i*3+1].ny,vb->glVertices[i*3+1].nz);
			glVertex3f(		vb->glVertices[i*3+1].x ,vb->glVertices[i*3+1].y ,vb->glVertices[i*3+1].z );
			glTexCoord2f(	vb->glVertices[i*3+2].tu,vb->glVertices[i*3+2].tv);
			glNormal3f(		vb->glVertices[i*3+2].nx,vb->glVertices[i*3+2].ny,vb->glVertices[i*3+2].nz);
			glVertex3f(		vb->glVertices[i*3+2].x ,vb->glVertices[i*3+2].y ,vb->glVertices[i*3+2].z );
		glEnd();
	}
#endif
	NixNumTrias+=vb->NumTrias;
}

void NixDraw3DCubeMapped(int cube_map,int buffer,const matrix &mat)
{
	if (buffer<0)	return;

#ifdef NIX_API_DIRECTX9
	if (NixApi==NIX_API_DIRECTX9){
		NixSetCubeMapDX(cube_map);
		lpDevice->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
		matrix tm=NixViewMatrix;
		tm._03=tm._13=tm._23=tm._33=tm._30=tm._31=tm._32=0;
		/*quaternion q;
		QuaternionRotationM(q,tm);
		vector ang=QuaternionToAngle(q);
		MatrixRotationView(tm,ang);*/
		MatrixTranspose(tm,tm);
		lpDevice->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT3);

		lpDevice->SetTransform(D3DTS_TEXTURE0,(D3DMATRIX*)&tm);


		NixDraw3D(-2,buffer,mat);
		lpDevice->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_PASSTHRU);
		lpDevice->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE);
	}
#endif
#ifdef NIX_API_OPENGL
	if (NixApi==NIX_API_OPENGL){
		msg_todo("Draw3DCubeMapped for OpenGL");

		//Draw3D(-1,buffer,mat);
	}
#endif
}

void NixDrawSpriteR(int texture,const color &col,const rect &src,const vector &pos,const rect &dest)
{
	rect d;
	float depth;
	vector p;
	NixGetVecProject(p,pos);
	if ((p.z<=0.0f)||(p.z>=1.0))
		return;
	depth=p.z;
	vector u;
	VecTransform(u,NixViewMatrix,pos);
	float q=NixMaxDepth/(NixMaxDepth-NixMinDepth);
	float f=1.0f/(u.z*q*NixMinDepth*NixView3DRatio);
#ifdef NIX_API_OPENGL
	if (NixApi==NIX_API_OPENGL){
		//depth=depth*2.0f-1.0f;
		//f*=2;
	}
#endif
	//if (f>20)	f=20;
	d.x1=p.x+f*(dest.x1)*NixViewScale.x*NixTargetWidth;
	d.x2=p.x+f*(dest.x2)*NixViewScale.x*NixTargetWidth;
	d.y1=p.y+f*(dest.y1)*NixViewScale.y*NixTargetHeight*NixView3DRatio;
	d.y2=p.y+f*(dest.y2)*NixViewScale.y*NixTargetHeight*NixView3DRatio;
	NixDraw2D(texture,col,src,d,depth);
}

void NixDrawSprite(int texture,const color &col,const rect &src,const vector &pos,float radius)
{
	rect d;
	d.x1=-radius;
	d.x2=radius;
	d.y1=-radius;
	d.y2=radius;
	NixDrawSpriteR(texture,col,src,pos,d);
}

void NixResetToColor(const color &c)
{
	glClearColor(c.r, c.g, c.b, c.a);
	glClear(GL_COLOR_BUFFER_BIT);
}

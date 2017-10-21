/*----------------------------------------------------------------------------*\
| Nix view                                                                     |
| -> camera etc...                                                             |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#include "nix.h"
#include "nix_common.h"
#ifdef _X_USE_IMAGE_
#include "../image/image.h"
#endif
#include "../hui/Controls/Control.h"
#include "../hui/Controls/ControlDrawingArea.h"

namespace nix{

void TestGLError(const string &);

void UpdateLights();


matrix view_matrix, projection_matrix;
matrix projection_matrix2d;
matrix world_matrix, world_view_projection_matrix;
vector _CamPos_;
bool mode3d = false;

float view_jitter_x = 0, view_jitter_y = 0;

static int OGLViewPort[4];

Texture *RenderingToTexture = NULL;

#ifdef OS_WINDOWS
	extern HDC hDC;
	extern HGLRC hRC;
#endif

void _SetMode2d()
{
	if (!mode3d)
		return;
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf((float*)&projection_matrix2d);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	mode3d = false;
	TestGLError("Set2d");
}

void _SetMode3d()
{
	/*if (NixMode3d)
		return;
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf((float*)&projection_matrix);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf((float*)&view_matrix);
	glMultMatrixf((float*)&NixWorldMatrix);
	NixMode3d = true;
	TestGLError("Set3d");*/
}

void Resize(int width, int height)
{
	target_width = max(width, 1);
	target_height = max(height, 1);
	target_rect = rect(0, (float)target_width, 0, (float)target_height);

	// screen
	glViewport(0, 0, target_width, target_height);
	OGLViewPort[0] = 0;
	OGLViewPort[1] = 0;
	OGLViewPort[2] = target_width;
	OGLViewPort[3] = target_height;
	TestGLError("glViewport");

	// camera
	//NixSetProjectionMatrix(projection_matrix);
	//NixSetViewMatrix(view_matrix);
}

void SetWorldMatrix(const matrix &mat)
{
	world_matrix = mat;
	world_view_projection_matrix = projection_matrix * view_matrix * world_matrix;
}

static vector ViewPos,ViewDir;
static vector Frustrum[8];
static plane FrustrumPl[6];

void SetViewPosAngV(const vector &view_pos,const vector &view_ang)
{
	quaternion q;
	QuaternionRotationV(q, view_ang);
	SetViewPosAng(view_pos, q);
}

void SetViewPosAng(const vector &view_pos,const quaternion &view_ang)
{
	ViewPos = view_pos;
	ViewDir = view_ang * e_z;

	matrix t, r;
	MatrixTranslation(t, -view_pos);
	MatrixRotationQ(r, view_ang);
	MatrixTranspose(r, r);
	view_matrix = r * t;
	SetViewMatrix(view_matrix);

	// die Eckpunkte des Sichtfeldes
	/*NixGetVecUnproject(Frustrum[0],vector(                   0,                    0,0.0f));
	NixGetVecUnproject(Frustrum[1],vector(float(NixScreenWidth-1),                    0,0.0f));
	NixGetVecUnproject(Frustrum[2],vector(                   0,float(NixScreenHeight-1),0.0f));
	NixGetVecUnproject(Frustrum[3],vector(float(NixScreenWidth-1),float(NixScreenHeight-1),0.0f));
	NixGetVecUnproject(Frustrum[4],vector(                   0,                    0,0.9f));
	NixGetVecUnproject(Frustrum[5],vector(float(NixScreenWidth-1),                    0,0.9f));
	NixGetVecUnproject(Frustrum[6],vector(                   0,float(NixScreenHeight-1),0.9f));
	NixGetVecUnproject(Frustrum[7],vector(float(NixScreenWidth-1),float(NixScreenHeight-1),0.9f));

	// Ebenen des Sichtfeldes (gegen UZS nach innen!?)
	PlaneFromPoints(FrustrumPl[0],Frustrum[0],Frustrum[1],Frustrum[2]); // nahe Ebene
	//PlaneFromPoints(FrustrumPl[1],Frustrum[4],Frustrum[6],Frustrum[7]); // ferne Ebene
	//PlaneFromPoints(FrustrumPl[2],Frustrum[0],Frustrum[2],Frustrum[3]); // linke Ebene
	//PlaneFromPoints(FrustrumPl[3],Frustrum[1],Frustrum[5],Frustrum[7]); // rechte Ebene
	//PlaneFromPoints(FrustrumPl[4],Frustrum[0],Frustrum[4],Frustrum[5]); // untere Ebene
	//PlaneFromPoints(FrustrumPl[5],Frustrum[2],Frustrum[3],Frustrum[7]); // obere Ebene*/
	TestGLError("SetView");
}

void create_pixel_projection_matrix(matrix &m)
{
	matrix s, t;
	MatrixTranslation(t, vector(-float(target_width)/2.0f,-float(target_height)/2.0f,0));
	MatrixScale(s, 2.0f / float(target_width), -2.0f / float(target_height), 1);
	m = s * t;
}

// 3D-Matrizen erstellen (Einstellungen ueber SetPerspectiveMode vor NixStart() zu treffen)
// enable3d: true  -> 3D-Ansicht auf (View3DWidth,View3DHeight) gemapt
//           false -> Pixel-Angaben~~~
// beide Bilder sind um View3DCenterX,View3DCenterY (3D als Fluchtpunkt) verschoben

void SetProjectionPerspective()
{
	SetProjectionPerspectiveExt((float)target_width / 2, (float)target_height / 2, (float)target_height, (float)target_height, 0.001f, 10000);
}

// center_x/y: pixel coordinates of perspective center
// height_1/width_1: pixel sizes of 45° frustrum
void SetProjectionPerspectiveExt(float center_x, float center_y, float width_1, float height_1, float z_min, float z_max)
{
	matrix trans, persp, scale;
	// perspective projection
	MatrixTranslation(trans,
		vector((center_x + view_jitter_x) / float(target_width) * 2.0f - 1,
			1 - (center_y + view_jitter_y) / float(target_height) * 2.0f,
			0));
	MatrixPerspective(persp, pi / 2, 1, z_min, z_max);
	MatrixScale(scale, 2 * width_1 / target_width,
			2 * height_1 / target_height,
			- 1); // z reflection: right/left handedness

	SetProjectionMatrix(trans * persp * scale);
}

// center_x/y: pixel coordinates of (0,0,0)
// map_width/height: pixel sizes of projected base vectors
void SetProjectionOrthoExt(float center_x, float center_y, float map_width, float map_height, float z_min, float z_max)
{
	matrix scale, trans;
	MatrixScale(scale, 2.0f / float(target_width) * map_width, -2.0f / float(target_height) * map_height, 2 / (z_max - z_min));
	MatrixTranslation(trans, vector(2 * center_x / target_width - 1, 1 - 2 * center_y / target_height, -(z_max + z_min) / (z_max - z_min)));
	SetProjectionMatrix(trans * scale);
}

void SetProjectionOrtho(bool relative)
{
	matrix m;
	if (relative){
		// orthogonal projection (relative [0,1]x[0x1] coordinates)
		matrix s, t;
		MatrixTranslation(t, vector(-0.5f, -0.5f, 0));
		MatrixScale(s, 2.0f, -2.0f, 1);
		m = s * t;
	}else{
		// orthogonal projection (pixel coordinates)
		//NixSetProjectionOrthoExt(0, 0, 1, 1, )
		create_pixel_projection_matrix(m);
	}

	SetProjectionMatrix(m);
}

void SetProjectionMatrix(const matrix &m)
{
	// projection 2d
	create_pixel_projection_matrix(projection_matrix2d);

	// projection 3d
	projection_matrix = m;
}

void SetViewMatrix(const matrix &m)
{
	view_matrix = m;

	//NixSetWorldMatrix(m_id);

	//NixUpdateLights();
}

#define FrustrumAngleCos	0.83f

bool IsInFrustrum(const vector &pos,float radius)
{
	// die absoluten Eckpunkte der BoundingBox
	vector p[8];
	p[0]=pos+vector(-radius,-radius,-radius);
	p[1]=pos+vector( radius,-radius,-radius);
	p[2]=pos+vector(-radius, radius,-radius);
	p[3]=pos+vector( radius, radius,-radius);
	p[4]=pos+vector(-radius,-radius, radius);
	p[5]=pos+vector( radius,-radius, radius);
	p[6]=pos+vector(-radius, radius, radius);
	p[7]=pos+vector( radius, radius, radius);

	bool in=false;
	for (int i=0;i<8;i++)
		//for (int j=0;j<6;j++)
			if (FrustrumPl[0].distance(p[i])<0)
				in=true;
	/*vector d;
	VecNormalize(d,pos-ViewPos); // zu einer Berechnung zusammenfassen!!!!!!
	float fdp=VecLengthFuzzy(pos-ViewPos);
	if (fdp<radius)
		return true;
	if (VecDotProduct(d,ViewDir)>FrustrumAngleCos-radius/fdp*0.04f)
		return true;
	return false;*/
	return in;
}

bool Rendering=false;


#ifdef OS_WINDOWS
	#ifdef HUI_API_GTK
		#include <gdk/gdkwin32.h>
	#endif
	extern HWND hWndSubWindow;
	extern bool nixDevNeedsUpdate;
#endif

bool Start()
{
	return StartIntoTexture(NULL);
}

bool StartIntoTexture(Texture *texture)
{
	if (DoingEvilThingsToTheDevice)
		return false;

	msg_db_f("NixStart", 2);
	TestGLError("Start prae");


#ifdef OS_WINDOWS
	if (nixDevNeedsUpdate){
		wglDeleteContext(hRC);
	PIXELFORMATDESCRIPTOR pfd={	sizeof(PIXELFORMATDESCRIPTOR),
								1,						// versions nummer
								PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
								PFD_TYPE_RGBA,
								32,//NixFullscreen?depth:NixDesktopDepth,
								//8, 0, 8, 8, 8, 16, 8, 24,
								0, 0, 0, 0, 0, 0, 0, 0, 0,
								0, 0, 0, 0,
								24,						// 24bit Z-Buffer
								8,						// 8bit stencil buffer
								0,						// no "Auxiliary"-buffer
								PFD_MAIN_PLANE,
								0, 0, 0, 0 };
		GtkWidget *gl_widget = NixWindow->_get_control_(NixControlID)->widget;
	
		hDC = GetDC((HWND)GDK_WINDOW_HWND(gtk_widget_get_window(gl_widget)));
		//hDC = GetDC(hWndSubWindow);
		if (!hDC){
			HuiErrorBox(NixWindow, "Fehler", "GetDC..." + i2s(GetLastError()));
			exit(0);
		}
		int OGLPixelFormat = ChoosePixelFormat(hDC, &pfd);
		SetPixelFormat(hDC, OGLPixelFormat, &pfd);
		hRC=wglCreateContext(hDC);
		if (!hRC){
			HuiErrorBox(NixWindow, "Fehler", "wglCreateContext...");
			exit(0);
		}
		int rr=wglMakeCurrent(hDC, hRC);
		if (rr != 1){
			HuiErrorBox(NixWindow, "Fehler", "wglMakeCurrent...");
			exit(0);
		}
		NixSetCull(CullDefault);
		nixDevNeedsUpdate = false;
	}
#endif

	NumTrias=0;
	RenderingToTexture=texture;
	//msg_write("Start " + p2s(texture));
	if (!texture){
		#ifdef OS_WINDOWS
	//		if (OGLDynamicTextureSupport)
	//			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
			/*if (!wglMakeCurrent(hDC,hRC)){
				msg_error("wglMakeCurrent");
				msg_write(GetLastError());
				return false;
			}*/
		#endif

	}else{
		if (OGLDynamicTextureSupport){

			glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, texture->glFrameBuffer );
			//glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, texture->glDepthRenderBuffer );
			glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texture->glTexture, 0 );
			glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, texture->glDepthRenderBuffer );
			GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
			if (status == GL_FRAMEBUFFER_COMPLETE_EXT){
				//msg_write("hurra");
			}else{
				msg_write("we're screwed! (NixStart with dynamic texture target)");
				return false;
			}
		}
	}
	TestGLError("Start 1");
	glClearColor(0.0f,0.0f,0.0f,0.0f);
	glDisable(GL_SCISSOR_TEST);
	//glClearStencil(0);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);
	TestGLError("Start 2a");
	glClear(GL_DEPTH_BUFFER_BIT);
	TestGLError("Start 2b");
	glClear(GL_STENCIL_BUFFER_BIT);
	//glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	//glClear(GL_COLOR_BUFFER_BIT);
	TestGLError("Start 2");

	// adjust target size
	if (!texture){
		if (Fullscreen){
			// fullscreen mode
			target_width = ScreenWidth;
			target_height = ScreenHeight;
		}else{
		}
	}else{
		// texture
		target_width = texture->width;
		target_height = texture->height;
	}
	Resize(target_width, target_height);
	Rendering = true;

	/*if (texture < 0)
		NixUpdateInput();*/

	//msg_write("-ok?");
	TestGLError("Start post");
	return true;
}

void Scissor(const rect &_r)
{
	bool enable_scissors = true;
	rect r = _r;
	if (r.x1 < 0){
		enable_scissors=false;
		r = target_rect;
	}
	if (enable_scissors)
		glEnable(GL_SCISSOR_TEST);
	else
		glDisable(GL_SCISSOR_TEST);
	glScissor((int)r.x1, target_height - (int)r.y2, (int)r.width(), (int)r.height());
	glClearDepth(1.0f);
	TestGLError("StartPart");
}

void End()
{
	if (!Rendering)
		return;
	msg_db_f("NixEnd", 2);
	TestGLError("End prae");
	Rendering = false;
	glDisable(GL_SCISSOR_TEST);
	if (!RenderingToTexture){
		// auf den Bildschirm
		#ifdef OS_WINDOWS
			if (RenderingToTexture<0)
				SwapBuffers(hDC);
		#endif
		#ifdef OS_LINUX
			#ifdef NIX_ALLOW_FULLSCREEN
				if (NixFullscreen)
					XF86VidModeSetViewPort(x_display,screen,0,NixDesktopHeight-NixScreenHeight);
			#endif
			//glutSwapBuffers();
			if (GLDoubleBuffered){
			}
		#endif
	}
	if (OGLDynamicTextureSupport)
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	ProgressTextureLifes();
	TestGLError("End post");
}

void SetClipPlane(int index,const plane &pl)
{
	GLdouble d[4];
	d[0]=pl.n.x;
	d[1]=pl.n.y;
	d[2]=pl.n.z;
	d[3]=pl.d;
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf((float*)&view_matrix);
	glClipPlane(GL_CLIP_PLANE0+index,d);
	glPopMatrix();
	//msg_todo("SetClipPlane fuer OpenGL");
	TestGLError("SetClip");
}

void EnableClipPlane(int index,bool enabled)
{
	if (enabled)
		glEnable(GL_CLIP_PLANE0+index);
	else
		glDisable(GL_CLIP_PLANE0+index);
	TestGLError("EnableClip");
}

void ScreenShot(const string &filename, int width, int height)
{
	Image image;
	int dx = target_width;
	int dy = target_height;
	image.data.resize(dx * dy);
	glReadBuffer(GL_FRONT);
	glReadPixels(	0,
					0,
					dx,
					dy,
					GL_RGBA, GL_UNSIGNED_BYTE, &image.data[0]);
	if ((width >= 0) && (height >= 0)){
		Array<unsigned int> data2;
		image.width = width;
		image.height = height;
		data2.resize(width * height);
		// flip image...
		for (int x=0;x<width;x++)
			for (int y=0;y<height;y++){
				int x1 = (x * dx) / width;
				int y1 = dy - (y * dy) / height - 1;
				int n1 = (x1 + dx * y1);
				int n2 = (x + width * y );
				data2[n2] = image.data[n1];
			}
		image.data.exchange((DynamicArray&)data2);
		data2.clear();
	}else{
		image.width = dx;
		image.height = dy;
		// flip image...
		for (int x=0;x<dx;x++)
			for (int y=0;y<(dy+1)/2;y++){
				int y2 = dy - y - 1;
				int n1 = (x + dx * y );
				int n2 = (x + dx * y2);
				int c = image.data[n1];
				image.data[n1] = image.data[n2];
				image.data[n2] = c;
			}
	}
	// set alpha to 1
	for (int i=0;i<image.data.num;i++)
		image.data[i] |= 0xff000000;
	// save
	image.save(filename);
	msg_write("screenshot saved: " + filename.sys_filename());
}

void ScreenShotToImage(Image &image)
{
	image.create(target_width, target_height, Black);
	glReadBuffer(GL_FRONT);
	glReadPixels(	0,
					0,
					target_width,
					target_height,
					GL_RGBA, GL_UNSIGNED_BYTE, &image.data[0]);
}



// world -> screen (0...target_width,0...target_height,0...1)
void GetVecProject(vector &vout,const vector &vin)
{
	vout = world_view_projection_matrix.project(vin);
	return;

	/*matrix m;
	MatrixIdentity(m);*/
	double vm[16];
	int i;
	for (i=0;i<16;i++)
		vm[i]=view_matrix.e[i];
		//vm[i]=m.e[i];
	double pm[16];
	for (i=0;i<16;i++)
			pm[i]=projection_matrix.e[i];
		//pm[i]=m.e[i];
	double x,y,z;
	gluProject(vin.x,vin.y,vin.z,vm,pm,OGLViewPort,&x,&y,&z);
	vout.x=(float)x;
	vout.y=float((OGLViewPort[1]*2+OGLViewPort[3])-y); // y-Spiegelung
	vout.z=(float)z;//0.999999970197677613f;//(float)z;
	/*VecTransform(vout,view_matrix,vin);
	VecTransform(vout,projection_matrix,vout);
	vout.y=((ViewPort[1]*2+ViewPort[3])-vout.y*16)/2;
	vout.x=((ViewPort[0]*2+ViewPort[2])+vout.x*16)/2;
	vout.z=0.99999997f;*/
	TestGLError("VecPro");
}

// world -> screen (0...1,0...1,0...1)
void GetVecProjectRel(vector &vout,const vector &vin)
{
	GetVecProject(vout,vin);
	vout.x/=(float)target_width;
	vout.y/=(float)target_height;
}

// screen (0...target_width,0...target_height,0...1) -> world
void GetVecUnproject(vector &vout,const vector &vin)
{
	double vin_y=OGLViewPort[1]*2+OGLViewPort[3]-(double)vin.y; // y-Spiegelung
	double vm[16];
	int i;
	for (i=0;i<16;i++)
		vm[i]=view_matrix.e[i];
	double pm[16];
	for (i=0;i<16;i++)
		pm[i]=projection_matrix.e[i];
	double x,y,z;
	gluUnProject(vin.x,vin_y,vin.z,vm,pm,OGLViewPort,&x,&y,&z);
	vout.x=(float)x;
	vout.y=(float)y;
	vout.z=(float)z;
	TestGLError("VecUnpro");
}

// screen (0...1,0...1,0...1) -> world
void GetVecUnprojectRel(vector &vout,const vector &vin)
{
	vector vi_r=vin;
	vi_r.x*=(float)target_width;
	vi_r.y*=(float)target_height;
	GetVecUnproject(vout,vi_r);
}

};

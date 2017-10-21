/*----------------------------------------------------------------------------*\
| Nix textures                                                                 |
| -> texture loading and handling                                              |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2008.11.09 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#include "nix.h"
#include "nix_common.h"
#include "../image/image.h"

namespace nix{

string texture_dir = "";

Array<Texture*> textures;
Texture *default_texture;

int TextureIconSize = 0;
bool OGLDynamicTextureSupport = true;
bool OGLMultiTexturingSupport = true;
bool GLDoubleBuffered = true;


void SetDefaultShaderData(int num_textures, const vector &cam_pos);
extern vector _CamPos_;



//--------------------------------------------------------------------------------------------------
// avi files
//--------------------------------------------------------------------------------------------------

#ifdef NIX_ALLOW_VIDEO_TEXTURE


struct s_avi_info
{
	AVISTREAMINFO		psi;										// Pointer To A Structure Containing Stream Info
	PAVISTREAM			pavi;										// Handle To An Open Stream
	PGETFRAME			pgf;										// Pointer To A GetFrame Object
	BITMAPINFOHEADER	bmih;										// Header Information For DrawDibDraw Decoding
	long				lastframe;									// Last Frame Of The Stream
	int					width;										// Video Width
	int					height;										// Video Height
	char				*pdata;										// Pointer To Texture Data
	unsigned char*		data;										// Pointer To Our Resized Image
	HBITMAP hBitmap;												// Handle To A Device Dependant Bitmap
	float time,fps;
	int ActualFrame;
	HDRAWDIB hdd;												// Handle For Our Dib
	HDC hdc;										// Creates A Compatible Device Context
}*avi_info[NIX_MAX_TEXTURES];


static void avi_flip(void* buffer,int w,int h)
{
	unsigned char *b = (BYTE *)buffer;
	char temp;
    for (int x=0;x<w;x++)
    	for (int y=0;y<h/2;y++){
    		temp=b[(x+(h-y-1)*w)*3+2];
    		b[(x+(h-y-1)*w)*3+2]=b[(x+y*w)*3  ];
    		b[(x+y*w)*3  ]=temp;

    		temp=b[(x+(h-y-1)*w)*3+1];
    		b[(x+(h-y-1)*w)*3+1]=b[(x+y*w)*3+1];
    		b[(x+y*w)*3+1]=temp;

    		temp=b[(x+(h-y-1)*w)*3  ];
    		b[(x+(h-y-1)*w)*3  ]=b[(x+y*w)*3+2];
    		b[(x+y*w)*3+2]=temp;
    	}
}

static int GetBestVideoSize(int s)
{
	return NixMaxVideoTextureSize;
}

void avi_grab_frame(int texture,int frame)									// Grabs A Frame From The Stream
{
	if (texture<0)
		return;
	if (!avi_info[texture])
		return;
	if (avi_info[texture]->ActualFrame==frame)
		return;
	int w=NixTextureWidth[texture];
	int h=NixTextureHeight[texture];
	msg_write(w);
	msg_write(h);
	avi_info[texture]->ActualFrame=frame;
	LPBITMAPINFOHEADER lpbi;									// Holds The Bitmap Header Information
	lpbi = (LPBITMAPINFOHEADER)AVIStreamGetFrame(avi_info[texture]->pgf, frame);	// Grab Data From The AVI Stream
	avi_info[texture]->pdata=(char *)lpbi+lpbi->biSize+lpbi->biClrUsed * sizeof(RGBQUAD);	// Pointer To Data Returned By AVIStreamGetFrame

	// Convert Data To Requested Bitmap Format
	DrawDibDraw (avi_info[texture]->hdd, avi_info[texture]->hdc, 0, 0, w, h, lpbi, avi_info[texture]->pdata, 0, 0, avi_info[texture]->width, avi_info[texture]->height, 0);
	

	//avi_flip(avi_info[texture]->data,w,h);	// Swap The Red And Blue Bytes (GL Compatability)

	// Update The Texture
	glBindTexture(GL_TEXTURE_2D,OGLTexture[texture]);
	glEnable(GL_TEXTURE_2D);
	glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, avi_info[texture]->data);
	//gluBuild2DMipmaps(GL_TEXTURE_2D,3,w,h,GL_RGB,GL_UNSIGNED_BYTE,AVIdata);
}

bool avi_open(int texture,LPCSTR szFile)
{
	msg_db_r("OpenAvi",1);
	avi_info[texture]->hdc = CreateCompatibleDC(0);
	avi_info[texture]->hdd = DrawDibOpen();

	// Opens The AVI Stream
	if (AVIStreamOpenFromFile(&avi_info[texture]->pavi, sys_str_f(szFile), streamtypeVIDEO, 0, OF_READ, NULL) !=0){
		msg_error("Failed To Open The AVI Stream");
		return false;
	}

	AVIStreamInfo(avi_info[texture]->pavi, &avi_info[texture]->psi, sizeof(avi_info[texture]->psi));						// Reads Information About The Stream Into psi
	avi_info[texture]->width=avi_info[texture]->psi.rcFrame.right-avi_info[texture]->psi.rcFrame.left;					// Width Is Right Side Of Frame Minus Left
	avi_info[texture]->height=avi_info[texture]->psi.rcFrame.bottom-avi_info[texture]->psi.rcFrame.top;

	avi_info[texture]->lastframe=AVIStreamLength(avi_info[texture]->pavi);
	avi_info[texture]->fps=float(avi_info[texture]->lastframe)/float(AVIStreamSampleToTime(avi_info[texture]->pavi,avi_info[texture]->lastframe)/1000.0f);

	avi_info[texture]->bmih.biSize = sizeof (BITMAPINFOHEADER);					// Size Of The BitmapInfoHeader
	avi_info[texture]->bmih.biPlanes = 1;											// Bitplanes
	avi_info[texture]->bmih.biBitCount = 24;										// Bits Format We Want (24 Bit, 3 Bytes)
//	avi_info[texture]->bmih.biWidth = AVI_TEXTURE_WIDTH;											// Width We Want (256 Pixels)
//	avi_info[texture]->bmih.biHeight = AVI_TEXTURE_HEIGHT;										// Height We Want (256 Pixels)
	avi_info[texture]->bmih.biCompression = BI_RGB;								// Requested Mode = RGB

	avi_info[texture]->hBitmap = CreateDIBSection (avi_info[texture]->hdc, (BITMAPINFO*)(&avi_info[texture]->bmih), DIB_RGB_COLORS, (void**)(&avi_info[texture]->data), NULL, 0);
	SelectObject (avi_info[texture]->hdc, avi_info[texture]->hBitmap);								// Select hBitmap Into Our Device Context (hdc)

	avi_info[texture]->pgf=AVIStreamGetFrameOpen(avi_info[texture]->pavi, NULL);						// Create The PGETFRAME	Using Our Request Mode
	if (avi_info[texture]->pgf==NULL){
		msg_error("Failed To Open The AVI Frame");
		return false;
	}

	NixTextureWidth[texture]=GetBestVideoSize(avi_info[texture]->width);
	NixTextureHeight[texture]=GetBestVideoSize(avi_info[texture]->height);
	msg_write(NixTextureWidth[texture]);
	msg_write(NixTextureHeight[texture]);

	avi_info[texture]->time=0;
	avi_info[texture]->ActualFrame=1;
	avi_grab_frame(texture,1);
	msg_db_l(1);
	return true;
}

void avi_close(int texture)
{
	if (!avi_info[texture])
		return;
	DeleteObject(avi_info[texture]->hBitmap);										// Delete The Device Dependant Bitmap Object
	DrawDibClose(avi_info[texture]->hdd);											// Closes The DrawDib Device Context
	AVIStreamGetFrameClose(avi_info[texture]->pgf);								// Deallocates The GetFrame Resources
	AVIStreamRelease(avi_info[texture]->pavi);										// Release The Stream
	//AVIFileExit();												// Release The File
}

#endif



//#endif

//--------------------------------------------------------------------------------------------------
// common stuff
//--------------------------------------------------------------------------------------------------

void TexturesInit()
{
	#ifdef NIX_ALLOW_VIDEO_TEXTURE
		// allow AVI textures
		AVIFileInit();
	#endif


	default_texture = new Texture;
	Image image;
	image.create(16, 16, White);
	default_texture->overwrite(image);
}

void ReleaseTextures()
{
	for (Texture *t: textures){
		glBindTexture(GL_TEXTURE_2D, t->glTexture);
		glDeleteTextures(1, &t->glTexture);
	}
}

void ReincarnateTextures()
{
	for (Texture *t: textures){
		glGenTextures(1, &t->glTexture);
		t->reload();
	}
}

void ProgressTextureLifes()
{
	for (Texture *t: textures)
		if (t->life_time >= 0){
			t->life_time ++;
			if (t->life_time >= TextureMaxFramesToLive)
				t->unload();
		}
}

Texture::Texture()
{
	filename = "-empty-";
	is_dynamic = false;
	is_cube_map = false;
	valid = true;
	life_time = 0;
#ifdef NIX_ALLOW_VIDEO_TEXTURE
	avi_info = NULL;
#endif
	glGenTextures(1, &glTexture);
	glFrameBuffer = 0;
	glDepthRenderBuffer = 0;
	width = height = 0;

	textures.add(this);
}

Texture::~Texture()
{
	unload();
}

void Texture::__init__()
{
	new(this) Texture;
}

void Texture::__delete__()
{
	this->~Texture();
}

Texture *LoadTexture(const string &filename)
{
	if (filename.num < 1)
		return NULL;
	for (Texture *t: textures)
		if (filename == t->filename)
			return t->valid ? t : NULL;

	// test existence
	if (!file_test_existence(texture_dir + filename)){
		msg_error("texture file does not exist: " + filename);
		return NULL;
	}

	Texture *t = new Texture;
	t->filename = filename;
	t->reload();
	return t;
}

void Texture::reload()
{
	msg_db_r("NixReloadTexture", 1);
	msg_write("loading texture: " + filename);

	string _filename = texture_dir + filename;

	// test the file's existence
	int h=_open(sys_str_f(_filename), 0);
	if (h<0){
		msg_error("texture file does not exist!");
		msg_db_l(1);
		return;
	}
	_close(h);

	#ifdef NIX_ALLOW_VIDEO_TEXTURE
		avi_info[texture]=NULL;
	#endif

	string extension = filename.extension();

// AVI
	if (extension == "avi"){
		//msg_write("avi");
		#ifdef NIX_ALLOW_VIDEO_TEXTURE
			avi_info[texture]=new s_avi_info;

			glBindTexture(GL_TEXTURE_2D, glTexture);
			glEnable(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			if (!avi_open(texture,SysFileName(t->Filename))){
				avi_info[texture]=NULL;
				msg_db_l(1);
				return;
			}

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t->Width, t->Height, 0, GL_RGB, GL_UNSIGNED_BYTE, avi_info[texture]->data);
		#else
			msg_error("Support for video textures is not activated!!!");
			msg_write("-> un-comment the NIX_ALLOW_VIDEO_TEXTURE definition in the source file \"00_config.h\" and recompile the program");
			msg_db_l(1);
			return;
		#endif
	}else{
		Image image;
		image.loadFlipped(_filename);
		overwrite(image);
	}
	life_time = 0;
	msg_db_l(1);
}

void OverwriteTexture__(Texture *t, int target, int subtarget, const Image &image)
{
	if (!t)
		return;
	msg_db_r("NixOverwriteTexture", 1);

	#ifdef NIX_ALLOW_VIDEO_TEXTURE
		avi_info[texture]=NULL;
	#endif

	image.setMode(Image::ModeRGBA);

	if (!image.error){
		//glEnable(target);
		glBindTexture(target, t->glTexture);
		TestGLError("OverwriteTexture a");
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
		TestGLError("OverwriteTexture b");
		if (t->is_dynamic){
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}else{
			//glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		TestGLError("OverwriteTexture c");
#ifdef GL_GENERATE_MIPMAP
		if (image.alpha_used) 
			glTexImage2D(subtarget, 0, GL_RGBA8, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data.data);
		else
			glTexImage2D(subtarget, 0, GL_RGB8, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data.data);
		TestGLError("OverwriteTexture d");
		if (!t->is_dynamic)
			glGenerateMipmap(GL_TEXTURE_2D);
		TestGLError("OverwriteTexture e");
#else
		if (image.alpha_used)
			gluBuild2DMipmaps(subtarget,4,image.width,image.height,GL_RGBA,GL_UNSIGNED_BYTE, image.data.data);
		else
			gluBuild2DMipmaps(subtarget,3,image.width,image.height,GL_RGBA,GL_UNSIGNED_BYTE, image.data.data);
#endif
					
	//msg_todo("32 bit textures for OpenGL");
		//gluBuild2DMipmaps(subtarget,4,NixImage.width,NixImage.height,GL_RGBA,GL_UNSIGNED_BYTE,NixImage.data);
		//glTexImage2D(subtarget,0,GL_RGBA8,128,128,0,GL_RGBA,GL_UNSIGNED_BYTE,NixImage.data);
		//glTexImage2D(subtarget,0,4,256,256,0,4,GL_UNSIGNED_BYTE,NixImage.data);
		t->width = image.width;
		t->height = image.height;
		if (TextureIconSize > 0){
			t->icon = *image.scale(TextureIconSize, TextureIconSize);
			t->icon.flipV();
		}
	}
	t->life_time = 0;
	msg_db_l(1);
}

void Texture::overwrite(const Image &image)
{
	OverwriteTexture__(this, GL_TEXTURE_2D, GL_TEXTURE_2D, image);
}

void Texture::unload()
{
	msg_db_r("NixUnloadTexture", 1);
	msg_write("unloading Texture: " + filename);
	glBindTexture(GL_TEXTURE_2D, glTexture);
	glDeleteTextures(1, (unsigned int*)&glTexture);
	life_time = -1;
	msg_db_l(1);
}

inline void refresh_texture(Texture *t)
{
	if (t){
		if (t->life_time < 0)
			t->reload();
		t->life_time = 0;
	}
}

void SetTexture(Texture *t)
{
	//refresh_texture(t);
	if (!t)
		t = default_texture;

	if (t->is_cube_map){
		glEnable(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_CUBE_MAP, t->glTexture);
	}else{
		glBindTexture(GL_TEXTURE_2D, t->glTexture);
		TestGLError("SetTex b");
		/*if (TextureIsDynamic[texture]){
			#ifdef OS_WONDOWS
			#endif
		}*/
	}
}

void SetTextures(Array<Texture*> &textures)
{
	/*for (int i=0;i<num_textures;i++)
		if (texture[i] >= 0)
			refresh_texture(texture[i]);*/

	for (int i=0;i<textures.num;i++){
		Texture *t = textures[i];
		if (!t)
			t = default_texture;
		glActiveTexture(GL_TEXTURE0+i);
		if (textures[i]->is_cube_map){
			glBindTexture(GL_TEXTURE_CUBE_MAP, textures[i]->glTexture);
		}else{
			glBindTexture(GL_TEXTURE_2D, textures[i]->glTexture);
		}
		//TestGLError("SetTex"+i2s(i));
	}
	TestGLError("SetTextures");
}

void SetTextureVideoFrame(int texture,int frame)
{
#ifdef NIX_ALLOW_VIDEO_TEXTURE
	if (texture<0)
		return;
	if (!avi_info[texture])
		return;
	if (frame>avi_info[texture]->lastframe)
		frame=0;
	avi_grab_frame(texture,frame);
	avi_info[texture]->time=float(frame)/avi_info[texture]->fps;
#endif
}

void TextureVideoMove(int texture,float elapsed)
{
#ifdef NIX_ALLOW_VIDEO_TEXTURE
	if (texture<0)
		return;
	if (!avi_info[texture])
		return;
	msg_write("<NixTextureVideoMove>");
	avi_info[texture]->time+=elapsed;
	if (avi_info[texture]->time*avi_info[texture]->fps>avi_info[texture]->lastframe)
		avi_info[texture]->time=0;
	avi_grab_frame(texture,int(avi_info[texture]->time*avi_info[texture]->fps));
	msg_write("</NixTextureVideoMove>");
#endif
}


DynamicTexture::DynamicTexture(int _width, int _height)
{
	msg_db_r("NixDynamicTexture", 1);
	msg_write(format("creating dynamic texture [%d x %d] ", _width, _height));
	if (!OGLDynamicTextureSupport)
		return;
	filename = "-dynamic-";
	width = _width;
	height = _height;
	is_dynamic = true;
	
	// create the render target stuff
	glGenFramebuffersEXT(1, &glFrameBuffer);
	glGenRenderbuffersEXT(1, &glDepthRenderBuffer);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, glDepthRenderBuffer);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width, height);

	// create the actual (dynamic) texture
	Image image;
	image.create(width, height, Black);
	overwrite(image);

	msg_db_l(1);

}

void DynamicTexture::__init__(int width, int height)
{
	new(this) DynamicTexture(width, height);
}

bool Texture::start_render()
{
	return StartIntoTexture(this);
}


static int NixCubeMapTarget[] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

CubeMap::CubeMap(int size)
{
	msg_db_r("NixCubeMap", 1);
	msg_write(format("creating cube map [ %d x %d x 6 ]", size, size));
	width = size;
	height = size;
	is_cube_map = true;
	filename = "-cubemap-";
	msg_db_l(1);
}

void CubeMap::__init__(int size)
{
	new(this) CubeMap(size);
}

void Texture::fill_cube_map(int side, Texture *source)
{
	if (!is_cube_map)
		return;
	if (!source)
		return;
	if (source->is_cube_map)
		return;
	Image image;
	image.load(texture_dir + source->filename);
	OverwriteTexture__(this, GL_TEXTURE_CUBE_MAP, NixCubeMapTarget[side], image);
}

void SetCubeMatrix(vector pos,vector ang)
{
	// TODO
#ifdef NIX_API_DIRECTX9
	if (NixApi==NIX_API_DIRECTX9){
		matrix t,r;
		MatrixTranslation(t,-pos);
		MatrixRotationView(r,ang);
		MatrixMultiply(NixViewMatrix,r,t);
		lpDevice->SetTransform(D3DTS_VIEW,(D3DXMATRIX*)&NixViewMatrix);
		lpDevice->Clear(0,NULL,D3DCLEAR_ZBUFFER,0,1.0f,0);
	}
#endif
}

void Texture::render_to_cube_map(vector &pos,callback_function *render_func,int mask)
{
	if (mask<1)	return;
	// TODO
#ifdef NIX_API_DIRECTX9
	if (NixApi==NIX_API_DIRECTX9){
		HRESULT hr;
		matrix vm=NixViewMatrix;

		hr=DXRenderToEnvMap[cube_map]->BeginCube(DXCubeMap[cube_map]);
		if (hr!=D3D_OK){
			msg_error(string("DXRenderToEnvMap: ",DXErrorMsg(hr)));
			return;
		}
		D3DXMatrixPerspectiveFovLH((D3DXMATRIX*)&NixProjectionMatrix,pi/2,1,NixMinDepth,NixMaxDepth);
		lpDevice->SetTransform(D3DTS_PROJECTION,(D3DXMATRIX*)&NixProjectionMatrix);
		MatrixInverse(NixInvProjectionMatrix,NixProjectionMatrix);
		NixTargetWidth=NixTargetHeight=CubeMapSize[cube_map];
		DXViewPort.X=DXViewPort.Y=0;
		DXViewPort.Width=DXViewPort.Height=CubeMapSize[cube_map];
		lpDevice->SetViewport(&DXViewPort);

		if (mask&1){
			DXRenderToEnvMap[cube_map]->Face(D3DCUBEMAP_FACE_POSITIVE_X,0);
			SetCubeMatrix(pos,vector(0,pi/2,0));
			render_func();
		}
		if (mask&2){
			DXRenderToEnvMap[cube_map]->Face(D3DCUBEMAP_FACE_NEGATIVE_X,0);
			SetCubeMatrix(pos,vector(0,-pi/2,0));
			render_func();
		}
		if (mask&4){
			DXRenderToEnvMap[cube_map]->Face(D3DCUBEMAP_FACE_POSITIVE_Y,0);
			SetCubeMatrix(pos,vector(-pi/2,0,0));
			render_func();
		}
		if (mask&8){
			DXRenderToEnvMap[cube_map]->Face(D3DCUBEMAP_FACE_NEGATIVE_Y,0);
			SetCubeMatrix(pos,vector(pi/2,0,0));
			render_func();
		}
		if (mask&16){
			DXRenderToEnvMap[cube_map]->Face(D3DCUBEMAP_FACE_POSITIVE_Z,0);
			SetCubeMatrix(pos,v0);
			render_func();
		}
		if (mask&32){
			DXRenderToEnvMap[cube_map]->Face(D3DCUBEMAP_FACE_NEGATIVE_Z,0);
			SetCubeMatrix(pos,vector(0,pi,0));
			render_func();
		}
		DXRenderToEnvMap[cube_map]->End(0);

		/*lpDevice->BeginScene();
		SetCubeMatrix(pos,vector(0,pi,0));
		render_func();
		//lpDevice->EndScene();
		End();*/

		NixViewMatrix=vm;
		NixSetView(true,NixViewMatrix);
	}
#endif
#ifdef NIX_API_OPENGL
	if (Api==NIX_API_OPENGL){
		msg_todo("RenderToCubeMap for OpenGL");
	}
#endif
}

};

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

string NixTextureDir = "";

Array<NixTexture> NixTextures;

int NixTextureIconSize = 0;


void NixReloadTexture(int texture);
void NixUnloadTexture(int texture);

void NixSetDefaultShaderData(int texture0, int texture1, int texture2, int texture3);



void TestGLError(const string &pos);


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

void NixTexturesInit()
{
	#ifdef NIX_ALLOW_VIDEO_TEXTURE
		// allow AVI textures
		AVIFileInit();
	#endif
}

void NixReleaseTextures()
{
	for (int i=0;i<NixTextures.num;i++){
		glBindTexture(GL_TEXTURE_2D,NixTextures[i].glTexture);
		glDeleteTextures(1,(unsigned int*)&NixTextures[i].glTexture);
	}
}

void NixReincarnateTextures()
{
	for (int i=0;i<NixTextures.num;i++){
		glGenTextures(1,&NixTextures[i].glTexture);
		NixReloadTexture(i);
	}
}

void NixProgressTextureLifes()
{
	for (int i=0;i<NixTextures.num;i++)
		if (NixTextures[i].life_time >= 0){
			NixTextures[i].life_time ++;
			if (NixTextures[i].life_time >= NixTextureMaxFramesToLive)
				NixUnloadTexture(i);
		}
}

int NixCreateEmptyTexture()
{
	NixTexture t;
	t.filename = "-empty-";
	t.is_dynamic = false;
	t.is_cube_map = false;
	t.valid = true;
	t.life_time = 0;
#ifdef NIX_ALLOW_VIDEO_TEXTURE
	t.avi_info = NULL;
#endif
	glGenTextures(1, &t.glTexture);

	NixTextures.add(t);
	
	return NixTextures.num - 1;
}

int NixLoadTexture(const string &filename)
{
	if (filename.num < 1)
		return -1;
	for (int i=0;i<NixTextures.num;i++)
		if (filename == NixTextures[i].filename)
			return NixTextures[i].valid ? i : -1;

	// test existence
	if (!file_test_existence(NixTextureDir + filename)){
		msg_error("texture file does not exist: " + filename);
		return -1;
	}

	int tex = NixCreateEmptyTexture();
	NixTextures[tex].filename = filename;
	NixReloadTexture(tex);
	return tex;
}

void NixReloadTexture(int texture)
{
	if (texture < 0)
		return;
	msg_db_r("NixReloadTexture", 1);
	msg_write("loading texture: " + NixTextures[texture].filename);

	NixTexture *t = &NixTextures[texture];
	string filename = NixTextureDir + t->filename;

	// test the file's existence
	int h=_open(sys_str_f(filename), 0);
	if (h<0){
		msg_error("texture file does not exist!");
		msg_db_l(1);
		return;
	}
	_close(h);

	#ifdef NIX_ALLOW_VIDEO_TEXTURE
		avi_info[texture]=NULL;
	#endif

	string extension = t->filename.extension();

// AVI
	if (extension == "avi"){
		//msg_write("avi");
		#ifdef NIX_ALLOW_VIDEO_TEXTURE
			avi_info[texture]=new s_avi_info;

			glBindTexture(GL_TEXTURE_2D,t->glTexture);
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
		image.LoadFlipped(filename);
		NixOverwriteTexture(texture, image);
	}
	t->life_time = 0;
	msg_db_l(1);
}

void NixOverwriteTexture__(int texture, int target, int subtarget, const Image &image)
{
	if (texture < 0)
		return;
	msg_db_r("NixOverwriteTexture", 1);

	NixTexture &t = NixTextures[texture];

	#ifdef NIX_ALLOW_VIDEO_TEXTURE
		avi_info[texture]=NULL;
	#endif

	image.SetMode(Image::ModeRGBA);

	if (!image.error){
		glEnable(target);
		glBindTexture(target, t.glTexture);
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
		if (t.is_dynamic){
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}else{
			//glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
#ifdef GL_GENERATE_MIPMAP
		if (!t.is_dynamic)
			glTexParameteri(target, GL_GENERATE_MIPMAP, GL_TRUE);
		if (image.alpha_used) 
			glTexImage2D(subtarget, 0, GL_RGBA8, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data.data);
		else
			glTexImage2D(subtarget, 0, GL_RGB8, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data.data);
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
		t.width = image.width;
		t.height = image.height;
		if (NixTextureIconSize > 0){
			t.icon = image;
			t.icon.Scale(NixTextureIconSize, NixTextureIconSize);
			t.icon.FlipV();
		}
	}
	TestGLError("OverwriteTexture");
	t.life_time = 0;
	msg_db_l(1);
}

void NixOverwriteTexture(int texture, const Image &image)
{
	NixOverwriteTexture__(texture, GL_TEXTURE_2D, GL_TEXTURE_2D, image);
}

void NixUnloadTexture(int texture)
{
	msg_db_r("NixUnloadTexture", 1);
	msg_write("unloading Texture: " + NixTextures[texture].filename);
	glBindTexture(GL_TEXTURE_2D,NixTextures[texture].glTexture);
	glDeleteTextures(1,(unsigned int*)&NixTextures[texture].glTexture);
	NixTextures[texture].life_time = -1;
	msg_db_l(1);
}

inline void refresh_texture(int texture)
{
	if (texture >= 0){
		if (NixTextures[texture].life_time < 0)
			NixReloadTexture(texture);
		NixTextures[texture].life_time = 0;
	}
}

int _nix_num_textures_activated_ = 0;

void unset_textures()
{
	// unset multitexturing
	/*if (OGLMultiTexturingSupport){
		for (int i=1;i<_nix_num_textures_activated_;i++){
			glActiveTexture(GL_TEXTURE0+i);
			glDisable(GL_TEXTURE_2D);
			glClientActiveTexture(GL_TEXTURE0+i);
			glDisableClientState( GL_TEXTURE_COORD_ARRAY );
		}
		glActiveTexture(GL_TEXTURE0);
		glClientActiveTexture(GL_TEXTURE0);
	}
	_nix_num_textures_activated_ = 1;*/
}

void NixSetTexture(int texture)
{
	refresh_texture(texture);
	unset_textures();
	if (texture>=0){
		if (NixTextures[texture].is_cube_map){
			glEnable(GL_TEXTURE_CUBE_MAP);
			glBindTexture(GL_TEXTURE_CUBE_MAP,NixTextures[texture].glTexture);
		}else{
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,NixTextures[texture].glTexture);
			/*if (TextureIsDynamic[texture]){
				#ifdef OS_WONDOWS
				#endif
			}*/
		}
	}else{
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_CUBE_MAP);
	}
	_nix_num_textures_activated_ = 1;
	TestGLError("SetTex");
	if (NixGLCurrentProgram > 0)
		NixSetDefaultShaderData(texture,-1,-1,-1);
}

void NixSetTextures(int *texture, int num_textures)
{
	for (int i=0;i<num_textures;i++)
		refresh_texture(texture[i]);
	unset_textures();

	// set multitexturing
	if (OGLMultiTexturingSupport){
		for (int i=0;i<num_textures;i++){
			glActiveTexture(GL_TEXTURE0+i);
			if (NixTextures[texture[i]].is_cube_map){
				glEnable(GL_TEXTURE_CUBE_MAP);
				glBindTexture(GL_TEXTURE_CUBE_MAP,NixTextures[texture[i]].glTexture);
			}else{
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,NixTextures[texture[i]].glTexture);
			}
		}
	}else{
		if (NixTextures[texture[0]].is_cube_map){
			glEnable(GL_TEXTURE_CUBE_MAP);
			glBindTexture(GL_TEXTURE_CUBE_MAP,NixTextures[texture[0]].glTexture);
		}else{
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,NixTextures[texture[0]].glTexture);
		}
	}

	_nix_num_textures_activated_ = num_textures;
	if (NixGLCurrentProgram > 0)
		NixSetDefaultShaderData(texture[0], texture[1], texture[2], texture[3]);
}

void NixSetTextureVideoFrame(int texture,int frame)
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

void NixTextureVideoMove(int texture,float elapsed)
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


int NixCreateDynamicTexture(int width,int height)
{
	msg_db_r("NixCreateDynamicTexture", 1);
	msg_write(format("creating dynamic texture [%d x %d] ", width, height));
	if (!OGLDynamicTextureSupport)
		return -1;
	int tex = NixCreateEmptyTexture();
	NixTexture &t = NixTextures[tex];
	t.filename = "-dynamic-";
	t.width = width;
	t.height = height;
	t.is_dynamic = true;
	
	// create the render target stuff
	glGenFramebuffersEXT(1, &t.glFrameBuffer);
	glGenRenderbuffersEXT(1, &t.glDepthRenderBuffer);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, t.glDepthRenderBuffer);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width, height);
	
	/*glGenFramebuffers(1, &t.glFrameBuffer);
	glGenRenderbuffers(1, &t.glDepthRenderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, t.glDepthRenderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);*/

	// create the actual (dynamic) texture
	Image image;
	image.Create(width, height, Black);
	NixOverwriteTexture(tex, image);

	msg_db_l(1);
	return tex;

}



static int NixCubeMapTarget[] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

int NixCreateCubeMap(int size)
{
	msg_db_r("NixCreateCubeMap", 1);
	msg_write(format("creating cube map [ %d x %d x 6 ]", size, size));
	int tex = NixCreateEmptyTexture();
	NixTexture &t = NixTextures[tex];
	t.width = size;
	t.height = size;
	t.is_cube_map = true;
	t.filename = "-cubemap-";
	msg_db_l(1);
	return tex;
}

void NixFillCubeMap(int cube_map, int side, int source_tex)
{
	if ((cube_map < 0) || (source_tex < 0))
		return;
	NixTexture &c = NixTextures[cube_map];
	NixTexture &s = NixTextures[source_tex];
	if (!c.is_cube_map)
		return;
	if (s.is_cube_map)
		return;
	Image image;
	image.LoadFlipped(NixTextureDir + s.filename);
	NixOverwriteTexture__(cube_map, GL_TEXTURE_CUBE_MAP, NixCubeMapTarget[side], image);
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

void NixRenderToCubeMap(int cube_map,vector &pos,callback_function *render_func,int mask)
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
	if (NixApi==NIX_API_OPENGL){
		msg_todo("RenderToCubeMap for OpenGL");
	}
#endif
}

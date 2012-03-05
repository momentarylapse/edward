/*----------------------------------------------------------------------------*\
| Nix vertex buffer                                                            |
| -> handling vertex buffers                                                   |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#include "nix.h"
#include "nix_common.h"

Array<sVertexBuffer> NixVB;


// bei angegebenem index wird der bestehende VB neu erstellt
int NixCreateVB(int max_trias,int index)
{
	msg_db_r("creating vertex buffer", 1);
	bool create = (index < 0);
	if (create){
		index = NixVB.num;
		for (int i=0;i<NixVB.num;i++)
			if (!NixVB[i].Used){
				index = i;
				break;
			}
		if (index == NixVB.num)
			NixVB.resize(NixVB.num + 1);
	}
	NixVB[index].glVertices = new vector[max_trias*3];
	NixVB[index].glNormals = new vector[max_trias*3];
	NixVB[index].glTexCoords[0] = new float[max_trias*6];
	if ((!NixVB[index].glVertices) || (!NixVB[index].glNormals) || (!NixVB[index].glTexCoords[0])){
		msg_error("couldn't create vertex buffer");
		msg_db_l(1);
		return -1;
	}
	#ifdef ENABLE_INDEX_BUFFERS
		/*VBIndex[NumVBs]=new ...; // noch zu bearbeiten...
		if (!OGLVBIndex[index]){
			msg_error("IndexBuffer konnte nicht erstellt werden");
			return -1;
		}*/
	#endif
	//msg_write(index);
	//msg_write(max_trias);
	NixVB[index].NumTextures = 1;
	NixVB[index].NumTrias = NixVB[index].NumPoints = 0;
	NixVB[index].MaxTrias = max_trias;
	//NixVB[index].MaxPoints = max_trias;
	NixVB[index].Used = true;
	NixVB[index].NotedFull = false;
	msg_db_l(1);
	return index;
}

int NixCreateVBM(int max_trias,int num_textures,int index)
{
	msg_db_r(format("creating vertex buffer (%d tex coords)",num_textures).c_str(), 1);
	bool create = (index < 0);
	if (num_textures > NIX_MAX_TEXTURELEVELS)
		num_textures = NIX_MAX_TEXTURELEVELS;
	if (create){
		index = NixVB.num;
		for (int i=0;i<NixVB.num;i++)
			if (!NixVB[i].Used){
				index = i;
				break;
			}
		if (index == NixVB.num)
			NixVB.resize(NixVB.num + 1);
	}
	NixVB[index].glVertices = new vector[max_trias*3];
	NixVB[index].glNormals = new vector[max_trias*3];
	bool failed = ((!NixVB[index].glVertices) || (!NixVB[index].glNormals));
	for (int i=0;i<num_textures;i++){
		NixVB[index].glTexCoords[i] = new float[max_trias*6];
		failed = failed || (!NixVB[index].glTexCoords[i]);
	}
	if (failed){
		msg_error("couldn't create vertex buffer");
		msg_db_l(1);
		return -1;
	}
	#ifdef ENABLE_INDEX_BUFFERS
		/*VBIndex[NumVBs]=new ...; // noch zu bearbeiten...
		if (!OGLVBIndex[index]){
			msg_error("IndexBuffer konnte nicht erstellt werden");
			return -1;
		}*/
	#endif
	NixVB[index].NumTextures = num_textures;
	NixVB[index].NumTrias = NixVB[index].NumPoints = 0;
	NixVB[index].MaxTrias = max_trias;
	//NixVB[buffer].MaxPoints[index]=max_trias;
	NixVB[index].Used = true;
	NixVB[index].NotedFull = false;
	msg_db_l(1);
	return index;
}

void NixDeleteVB(int buffer)
{
	if (buffer < 0)
		return;
	if (!NixVB[buffer].Used)
		return;
	msg_db_r("deleting vertex buffer", 1);
	//msg_write(buffer);
	delete[](NixVB[buffer].glVertices);
	delete[](NixVB[buffer].glNormals);
	for (int i=0;i<NixVB[buffer].NumTextures;i++)
		delete[](NixVB[buffer].glTexCoords[i]);
	NixVB[buffer].Used = false;
	msg_db_l(1);
}

bool NixVBAddTria(int buffer,	const vector &p1,const vector &n1,float tu1,float tv1,
								const vector &p2,const vector &n2,float tu2,float tv2,
								const vector &p3,const vector &n3,float tu3,float tv3)
{
	if (NixVB[buffer].NumTrias > NixVB[buffer].MaxTrias){
		if (!NixVB[buffer].NotedFull){
			msg_error("too many triangles in the vertex buffer!");
			msg_write(buffer);
			NixVB[buffer].NotedFull = true;
		}
		return false;
	}
	//msg_write("VertexBufferAddTriangle");
	NixVB[buffer].glVertices[NixVB[buffer].NumTrias*3  ]=p1;
	NixVB[buffer].glVertices[NixVB[buffer].NumTrias*3+1]=p2;
	NixVB[buffer].glVertices[NixVB[buffer].NumTrias*3+2]=p3;
	NixVB[buffer].glNormals[NixVB[buffer].NumTrias*3  ]=n1;
	NixVB[buffer].glNormals[NixVB[buffer].NumTrias*3+1]=n2;
	NixVB[buffer].glNormals[NixVB[buffer].NumTrias*3+2]=n3;
	NixVB[buffer].glTexCoords[0][NixVB[buffer].NumTrias*6  ]=tu1;
	NixVB[buffer].glTexCoords[0][NixVB[buffer].NumTrias*6+1]=1-tv1;
	NixVB[buffer].glTexCoords[0][NixVB[buffer].NumTrias*6+2]=tu2;
	NixVB[buffer].glTexCoords[0][NixVB[buffer].NumTrias*6+3]=1-tv2;
	NixVB[buffer].glTexCoords[0][NixVB[buffer].NumTrias*6+4]=tu3;
	NixVB[buffer].glTexCoords[0][NixVB[buffer].NumTrias*6+5]=1-tv3;
	NixVB[buffer].NumTrias ++;
	NixVB[buffer].Indexed = false;
	return true;
}

bool NixVBAddTriaM(int buffer,	const vector &p1,const vector &n1,const float *t1,
								const vector &p2,const vector &n2,const float *t2,
								const vector &p3,const vector &n3,const float *t3)
{
	if (buffer<0)	return false;
	if (NixVB[buffer].NumTrias > NixVB[buffer].MaxTrias){
		if (!NixVB[buffer].NotedFull){
			msg_error("too many triangles in the vertex buffer!");
			NixVB[buffer].NotedFull = true;
		}
		return false;
	}
	int n = NixVB[buffer].NumTrias;
	NixVB[buffer].glVertices[n*3  ]=p1;
	NixVB[buffer].glVertices[n*3+1]=p2;
	NixVB[buffer].glVertices[n*3+2]=p3;
	NixVB[buffer].glNormals[n*3  ]=n1;
	NixVB[buffer].glNormals[n*3+1]=n2;
	NixVB[buffer].glNormals[n*3+2]=n3;
	for (int i=0;i<NixVB[buffer].NumTextures;i++){
		NixVB[buffer].glTexCoords[i][n*6  ]=t1[i*2  ];
		NixVB[buffer].glTexCoords[i][n*6+1]=1-t1[i*2+1];
		NixVB[buffer].glTexCoords[i][n*6+2]=t2[i*2  ];
		NixVB[buffer].glTexCoords[i][n*6+3]=1-t2[i*2+1];
		NixVB[buffer].glTexCoords[i][n*6+4]=t3[i*2  ];
		NixVB[buffer].glTexCoords[i][n*6+5]=1-t3[i*2+1];
	}
	NixVB[buffer].NumTrias ++;
	NixVB[buffer].Indexed = false;
	return true;
}

// for each triangle there have to be 3 vertices (p[i],n[i],t[i*2],t[i*2+1])
void NixVBAddTrias(int buffer,int num_trias,const vector *p,const vector *n,const float *t)
{
	memcpy(NixVB[buffer].glVertices, p, sizeof(vector) * num_trias * 3);
	memcpy(NixVB[buffer].glNormals, n, sizeof(vector) * num_trias * 3);
	//memcpy(OGLVBTexCoords[buffer][0],t,sizeof(float)*num_trias*6);
	for (int i=0;i<num_trias*3;i++){
		NixVB[buffer].glTexCoords[0][i*2  ] = t[i*2];
		NixVB[buffer].glTexCoords[0][i*2+1] = t[i*2+1];
	}
	NixVB[buffer].NumTrias += num_trias;
	NixVB[buffer].NumPoints += num_trias * 3;
}

void NixVBAddTriasIndexed(int buffer,int num_points,int num_trias,const vector *p,const vector *n,const float *tu,const float *tv,const unsigned short *indices)
{
	#ifdef ENABLE_INDEX_BUFFERS
	#endif
}

void NixVBClear(int buffer)
{
	NixVB[buffer].NumTrias = 0;
	NixVB[buffer].NumPoints = 0;
	NixVB[buffer].Indexed = false;
	NixVB[buffer].NotedFull = false;
}

int NixVBGetMaxTrias(int buffer)
{
	if (buffer < 0)
		return 0;
	return NixVB[buffer].MaxTrias;
}


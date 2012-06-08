/*
 * DataModel.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef DATAMODEL_H_
#define DATAMODEL_H_

#include "../Data.h"
#include "../../MultiView.h"
#include "../../lib/x/x.h"
#include "ModeModelSurface.h"
#include "ModeModelMaterial.h"

class DataModel;
class ModeModelSurface;
class ModeModelMaterial;


#define TransparencyModeDefault			-1
#define TransparencyModeNone			0
#define TransparencyModeFunctions		1
#define TransparencyModeColorKeyHard	2
#define TransparencyModeColorKeySmooth	3
#define TransparencyModeFactor			4

// effects
enum{
	FXKindScript,
	FXKindLight,
	FXKindFog,
	FXKindSound,
	FXKindForceField
};

struct ModeModelFX{
	int Kind, Surface, Vertex;
	int Size, Colors[3][4],Speed,Intensity;
	bool InvQuad;
	string File, Function;
};

class ModeModelVertex: public MultiViewSingleData
{
public:
	int NormalMode;
	int BoneIndex;

	// vertex animation (all frames of all moves continuously)
	// TODO model.h

	bool NormalDirty;
	int RefCount; // triangles
	int Surface;
};

class ModeModelTriangle: public MultiViewSingleData
{
public:
	int Vertex[3], Edge[3];
	vector SkinVertex[MODEL_MAX_TEXTURES][3];
	int NormalIndex[3];
	vector Normal[3];
	vector TempNormal;
	bool NormalDirty;
	int Material;
};

// only for use in MultiView...
class ModeModelSkinVertexDummy: public MultiViewSingleData
{
};

class ModeModelBall: public MultiViewSingleData
{
public:
	int Index;
	float Radius;
};

struct ModeModelPolyhedronFace
{
	int NumVertices;
	int Index[MODEL_MAX_POLY_VERTICES_PER_FACE];
	plane Plane;
};


// TODO: dynamical!
class ModeModelPolyhedron: public MultiViewSingleData
{
public:
	int NumFaces;
	ModeModelPolyhedronFace Face[MODEL_MAX_POLY_FACES];
	int NumSVertices;
	int SIndex[MODEL_MAX_POLY_FACES * MODEL_MAX_POLY_VERTICES_PER_FACE];

	// non redundant edge list!
	int NumEdges;
	int EdgeIndex[MODEL_MAX_POLY_EDGES * 2];

	// "topology"
	bool EdgeOnFace[MODEL_MAX_POLY_EDGES * MODEL_MAX_POLY_FACES]; // [edge * NumFaces + face]
	int FacesJoiningEdge[MODEL_MAX_POLY_FACES * MODEL_MAX_POLY_FACES]; // [face1 * NumFaces + face2]
};


// triangles belonging to one material
struct ModeModelSubSkin
{
	int NumTextures; // "read only" (updated automatically...)

	// triangles
	Array<ModeModelTriangle> Triangle;
};


// geometry
struct ModeModelSkin
{
	// general properties
	int NormalModeAll;

	// vertices
	Array<ModeModelVertex> Vertex;

	// sub skins
	Array<ModeModelSubSkin> Sub;
};

// current geometry
//extern ModeModelSkin *skin;

/*struct PseudoSkin{
	int NumVertices;
	ModeModelVertex *Vertex;
	int NumTriangles;
	ModeModelTriangle *Triangle;
};*/

class ModeModelSkeletonBone: public MultiViewSingleData
{
public:
	int Parent;
	vector DeltaPos;
	string ModelFile;
	CModel *model;
	bool ConstPos;

	// for editing
	matrix Matrix, RotMatrix;
};

struct ModeModelFrame
{
	// skeleton animation
	Array<vector> SkelDPos;
	Array<vector> SkelAng;

	// vertex animation
	struct{
		Array<vector> DPos;
	}Skin[4];
	Array<vector> VertexDPos;
};

struct ModeModelMove
{
	int Type;
	Array<ModeModelFrame> Frame;
	float FramesPerSecConst, FramesPerSecFactor;
	bool InterpolatedQuadratic, InterpolatedLoop;
	string Name;
};

class ModeModelEdge: public MultiViewSingleData
{
public:
	int NormalMode;
	int Vertex[2];
	int RefCount, Triangle[2];
	bool IsRound; // for editing
};


struct ModeModelGeometry
{
	Array<ModeModelVertex> Vertex;
	Array<ModeModelTriangle> Triangle;
};


class DataModel: public Data
{
public:
	DataModel();
	virtual ~DataModel();

	void Reset();
	bool Load(const string &_filename, bool deep = true);
	bool Save(const string &_filename);

	void DebugShow();

	void SetNormalsDirtyByVertices(const Array<int> &index);
	void UpdateNormals();



	int GetNumMarkedVertices();
	int GetNumMarkedSkinVertices();
	int GetNumMarkedTriangles();
	int GetNumMarkedSurfaces();
	//int GetNumMarkedBalls();
	//int GetNumMarkedKonvPolys();

	void ClearSelection();
	void SelectionTrianglesFromVertices();
	void SelectionSurfacesFromTriangles();
	void SelectionTrianglesFromSurfaces();
	void SelectionVerticesFromTriangles();
	void SelectionVerticesFromSurfaces();


	float GetDiameter();
	void GenerateDetailDists(bool just_temp = false);
	void GenerateInertiaTensor(float mass, bool just_temp = false);
	void CreateSkin(ModeModelSkin *src, ModeModelSkin *dst, float quality_factor);

	void ResetAutoTexturing();
	void ApplyAutoTexturing(int a, int b, int c, vector *sv);


	struct sAutoTexturingData
	{
		bool enabled;
		// linear
		vector p0, dir_u, dir_v;

		int prev_material;
	};
	sAutoTexturingData AutoTexturingData;

	// low level (un-action'ed)
	//void LowLevelAddVertex(const vector &vd);
	ModeModelSurface *AddSurface(int surf_no = -1);
	ModeModelSurface *SurfaceJoin(ModeModelSurface *a, ModeModelSurface *b);
	int get_surf_no(ModeModelSurface *s);

	// high level (actions)
	void AddVertex(const vector &v);
	ModeModelTriangle *AddTriangle(int a, int b, int c);


	// properties
	Array<ModeModelSkeletonBone> Bone;


	// properties
	Array<ModeModelMove> Move;
	ModeModelMove *move;

	int CurrentMove,CurrentFrame;

	bool Playing,PlayLoop;
	float TimeScale,TimeParam,SimFrame,TimeInFrame;
	int Frame0,Frame1;

	bool ShowSkeleton;


	bool TextureMVNeedsUpdate;
	int NewRootPoint;

	// geometry
	Array<ModeModelVertex> Vertex;
	Array<ModeModelSurface> Surface;
	Array<MultiViewSingleData> SkinVertex; // only temporary...
	int SkinVertMat, SkinVertTL;

	// old geometry
	ModeModelSkin Skin[4];

	// geometry (physical)
	Array<ModeModelBall> Ball;
	Array<ModeModelPolyhedron> Poly;

	// general properties
	Array<ModeModelMaterial> Material;
	int CurrentMaterial, CurrentTextureLevel;
	float DetailDist[3];
	int DetailFactor[3];
	bool AutoGenerateDists,AutoGenerateSkin[3];
	bool AutoGenerateTensor;
	matrix3 InertiaTensor;
	vector Min, Max;

	// effects
	Array<ModeModelFX> Fx;



	// object-properties
	string Name, Description, ScriptFile;
	float Mass;
	float Radius;
	bool ActivePhysics, PassivePhysics;
	Array<string> Inventary;

	Array<float> ScriptVar;




	// editor
	//int SubMode, CreationMode, EditMode;
	int ViewStage;

	bool ForceNewNormals[4];
	bool NormalIndicesOK[4];

	// temporary data (copy/paste)
/*	Array<ModeModelVertex> BVertex;
	Array<ModeModelTriangle> BTriangle;
	Array<int> BTriangleMat;
	Array<ModeModelBall> BBall;
	Array<ModeModelPolyhedron> BPoly;*/
};

#endif /* DATAMODEL_H_ */

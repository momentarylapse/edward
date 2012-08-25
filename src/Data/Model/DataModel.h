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
	int Size, Speed, Intensity;
	color Colors[3];
	bool InvQuad;
	string File, Function;
};

class ModeModelVertex: public MultiViewSingleData
{
public:
	int NormalMode;
	int BoneIndex;

	vector AnimatedPos;

	bool NormalDirty;
	int RefCount; // triangles
	int Surface;
};

class ModeModelTriangle: public MultiViewSingleData
{
public:
	int Vertex[3];
	int Edge[3];
	int EdgeDirection[3]; // which no of triangle in edge's list are we?
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

extern ModeModelMove *EmptyMove;

class ModeModelEdge: public MultiViewSingleData
{
public:
	//int NormalMode;
	int Vertex[2];
	int RefCount, Triangle[2], Side[2];
	bool IsRound; // for editing
	float Weight; // for easify'ing

	// constraints:
	//  Vertex[0] = surf.Triangle[Triangle[0]].Vertex[Side[0]]
	//  Vertex[1] = surf.Triangle[Triangle[0]].Vertex[(Side[0] + 1) % 3]
	//  same for Triangle/Side[1] but Vertex[0 <-> 1]
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
	void SetAllNormalsDirty();
	void UpdateNormals();



	int GetNumMarkedVertices();
	int GetNumMarkedSkinVertices();
	int GetNumMarkedTriangles();
	int GetNumMarkedSurfaces();
	//int GetNumMarkedBalls();
	//int GetNumMarkedKonvPolys();
	int GetNumMarkedBones();

	void ClearSelection();
	void SelectionTrianglesFromVertices();
	void SelectionSurfacesFromTriangles();
	void SelectionTrianglesFromSurfaces();
	void SelectionVerticesFromTriangles();
	void SelectionVerticesFromSurfaces();


	float GetDiameter();
	void GetBoundingBox(vector &min, vector &max);
	void GenerateDetailDists(float *dist);
	matrix3 GenerateInertiaTensor(float mass);
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
	void AddVertex(const vector &pos, int bone_index = 0, int normal_mode = -1);
	ModeModelTriangle *AddTriangle(int a, int b, int c);
	ModeModelSurface *AddBall(const vector &_pos, float _radius, int _num_x, int _num_y, bool _as_sphere);
	ModeModelSurface *AddPlane(const vector &_pos, const vector &_dv1, const vector &_dv2, int _num_x, int _num_y);
	ModeModelSurface *AddCube(const vector &_pos, const vector &_dv1, const vector &_dv2, const vector &_dv3, int num_1, int num_2, int num_3);
	ModeModelSurface *AddCylinder(Array<vector> &pos, Array<float> &radius, int rings, int edges, bool closed);


	// properties
	Array<ModeModelSkeletonBone> Bone;


	// properties
	Array<ModeModelMove> Move;
	ModeModelMove *move;

	int CurrentMove,CurrentFrame;
	void SetCurrentMove(int move);
	void SetCurrentFrame(int frame);
	void SetCurrentFrameNext();
	void SetCurrentFramePrevious();
	void UpdateAnimation();
	void UpdateSkeleton();
	vector GetBonePos(int index);
	vector GetBonePosAnimated(int index);

	// actions
	void AddAnimation(int index, int type);
	void DeleteAnimation(int index);
	void AnimationAddFrame(int index, int frame);
	void AnimationDeleteFrame(int index, int frame);
	void AnimationDeleteCurrentFrame();
	void AnimationDuplicateCurrentFrame();

	bool Playing,PlayLoop;
	float TimeScale,TimeParam,SimFrame;

	bool ShowSkeleton;


	bool TextureMVNeedsUpdate;
	int NewRootPoint;

	// geometry
	Array<ModeModelVertex> Vertex;
	Array<ModeModelSurface> Surface;
	int NormalModeAll;
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
	vector Min, Max;

	// effects
	Array<ModeModelFX> Fx;

	struct MetaData
	{
		// level of detail
		float DetailDist[3];
		int DetailFactor[3];
		bool AutoGenerateDists,AutoGenerateSkin[3];

		// physics
		float Mass;
		bool ActivePhysics, PassivePhysics;
		bool AutoGenerateTensor;
		matrix3 InertiaTensor;

		// object-properties
		string Name, Description;
		Array<string> Inventary;

		// script
		string ScriptFile;
		Array<float> ScriptVar;

		void Reset();
	};
	MetaData meta_data;



	float Radius;




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

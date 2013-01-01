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
#include "../../lib/base/set.h"
#include "../../lib/x/x.h"
#include "ModelPolygon.h"
#include "ModelSurface.h"
#include "ModelMaterial.h"

class DataModel;
class ModelSurface;
class ModelMaterial;


class GeometryException
{
public:
	GeometryException(const string &_message){	message = _message;	}
	string message;
};

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

struct ModelEffect{
	int Kind, Surface, Vertex;
	float Size, Speed, Intensity;
	color Colors[3];
	bool InvQuad;
	string File;

	void clear();
	string get_type();
};

class ModelVertex: public MultiViewSingleData
{
public:
	int NormalMode;
	int BoneIndex;

	vector AnimatedPos;

	bool NormalDirty;
	int RefCount; // polygons
	int Surface;
};

// only for use in MultiView...
class ModelSkinVertexDummy: public MultiViewSingleData
{
};

class ModelBall: public MultiViewSingleData
{
public:
	int Index;
	float Radius;
};

struct ModelPolyhedronFace
{
	int NumVertices;
	int Index[MODEL_MAX_POLY_VERTICES_PER_FACE];
	plane Plane;
};


// TODO: dynamical!
class ModelPolyhedron: public MultiViewSingleData
{
public:
	int NumFaces;
	ModelPolyhedronFace Face[MODEL_MAX_POLY_FACES];
	int NumSVertices;
	int SIndex[MODEL_MAX_POLY_FACES * MODEL_MAX_POLY_VERTICES_PER_FACE];

	// non redundant edge list!
	int NumEdges;
	int EdgeIndex[MODEL_MAX_POLY_EDGES * 2];

	// "topology"
	bool EdgeOnFace[MODEL_MAX_POLY_EDGES * MODEL_MAX_POLY_FACES]; // [edge * NumFaces + face]
	int FacesJoiningEdge[MODEL_MAX_POLY_FACES * MODEL_MAX_POLY_FACES]; // [face1 * NumFaces + face2]
};

class ModelTriangle: public MultiViewSingleData
{
public:
	int Vertex[3];
	//int Edge[3];
	//int EdgeDirection[3]; // which no of triangle in edge's list are we?
	vector SkinVertex[MODEL_MAX_TEXTURES][3];
	int NormalIndex[3];
	vector Normal[3];
	vector TempNormal;
	bool NormalDirty;
	int Material;
};


// triangles belonging to one material
struct ModelSubSkin
{
	int NumTextures; // "read only" (updated automatically...)

	// triangles
	Array<ModelTriangle> Triangle;
};


// geometry
struct ModelSkin
{
	// general properties
	int NormalModeAll;

	// vertices
	Array<ModelVertex> Vertex;

	// sub skins
	Array<ModelSubSkin> Sub;
};

class ModelBone: public MultiViewSingleData
{
public:
	int Parent;
	vector DeltaPos;
	string ModelFile;
	Model *model;
	bool ConstPos;

	// for editing
	matrix Matrix, RotMatrix;
};

struct ModelFrame
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

struct ModelMove
{
	int Type;
	Array<ModelFrame> Frame;
	float FramesPerSecConst, FramesPerSecFactor;
	bool InterpolatedQuadratic, InterpolatedLoop;
	string Name;
};

class ModelEdge: public MultiViewSingleData
{
public:
	//int NormalMode;
	int Vertex[2];
	int RefCount, Polygon[2], Side[2];
	bool IsRound; // for editing
	float Weight; // for easify'ing

	// constraints:
	//  Vertex[0] = surf.Polygon[Triangle[0]].Vertex[Side[0]]
	//  Vertex[1] = surf.Polygon[Triangle[0]].Vertex[(Side[0] + 1) % 3]
	//  same for Polygon/Side[1] but Vertex[0 <-> 1]
};


struct ModelGeometry
{
	Array<ModelVertex> Vertex;
	Array<ModelPolygon> Polygon;
};

class ModelSelectionState
{
public:
	Set<int> Vertex;
	Set<int> Surface;
	Array<Set<int> > Polygon;
	Array<Set<int> > Edge;
	void clear();
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
	bool TestSanity(const string &loc);

	void SetNormalsDirtyByVertices(const Array<int> &index);
	void SetAllNormalsDirty();
	void UpdateNormals();



	int GetNumSelectedVertices();
	int GetNumSelectedSkinVertices();
	int GetNumSelectedPolygons();
	int GetNumSelectedSurfaces();
	//int GetNumSelectedBalls();
	//int GetNumSelectedPolys();
	int GetNumSelectedBones();
	int GetNumPolygons();

	void ClearSelection();
	void SelectionFromVertices();
	void SelectionFromPolygons();
	void SelectionFromSurfaces();
	void GetSelectionState(ModelSelectionState &s);
	void SetSelectionState(ModelSelectionState &s);
	Set<int> GetSelectedSurfaces();
	Set<int> GetSelectedVertices();
	void SelectOnlySurface(ModelSurface *s);


	float GetDiameter();
	void GetBoundingBox(vector &min, vector &max);
	void GenerateDetailDists(float *dist);
	matrix3 GenerateInertiaTensor(float mass);
	void CreateSkin(ModelSkin *src, ModelSkin *dst, float quality_factor);

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
	ModelSurface *AddSurface(int surf_no = -1);
	ModelSurface *SurfaceJoin(ModelSurface *a, ModelSurface *b);
	int get_surf_no(ModelSurface *s);

	// high level (actions)
	void AddVertex(const vector &pos, int bone_index = 0, int normal_mode = -1);
	ModelPolygon *AddTriangle(int a, int b, int c);
	ModelPolygon *AddPolygon(Array<int> &v);
	ModelSurface *AddBall(const vector &pos, float radius, int num_x, int num_y);
	ModelSurface *AddSphere(const vector &pos, float radius, int num);
	ModelSurface *AddPlane(const vector &pos, const vector &dv1, const vector &dv2, int num_x, int num_y);
	ModelSurface *AddCube(const vector &pos, const vector &dv1, const vector &dv2, const vector &dv3, int num_1, int num_2, int num_3);
	ModelSurface *AddCylinder(Array<vector> &pos, Array<float> &radius, int rings, int edges, bool closed);
	ModelSurface *AddTorus(const vector &pos, const vector &axis, float radius1, float radius2, int num_x, int num_y);

	void DeleteSelection(bool greedy = false);
	void InvertSelection();
	void InvertSurfaces(const Set<int> &surfaces);
	void SubtractSelection();
	void CutOutSelection();
	void TriangulateSelection();
	void AlignToGridSelection(float grid_d);
	void NearifySelectedVertices();
	void CollapseSelectedVertices();
	void BevelSelectedVertices(float radius);
	void FlattenSelectedVertices();
	void ExtrudeSelectedPolygons(float offset);
	void AutoWeldSurfaces(const Set<int> &surfaces, float d);
	void AutoWeldSelectedSurfaces(float d);
	void SetNormalModeSelection(int mode);
	void SetNormalModeAll(int mode);
	void SetMaterialSelection(int material);
	void CopyGeometry(ModelGeometry &geo); // not an action...
	void PasteGeometry(ModelGeometry &geo);
	void Easify(float factor);


	// properties
	Array<ModelBone> Bone;


	// properties
	Array<ModelMove> Move;
	ModelMove *move;

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
	Array<ModelVertex> Vertex;
	Array<ModelSurface> Surface;
	int NormalModeAll;
	Array<MultiViewSingleData> SkinVertex; // only temporary...
	int SkinVertMat, SkinVertTL;

	// old geometry
	ModelSkin Skin[4];

	// geometry (physical)
	Array<ModelBall> Ball;
	Array<ModelPolyhedron> Poly;

	// general properties
	Array<ModelMaterial> Material;
	int CurrentMaterial, CurrentTextureLevel;
	vector Min, Max;

	// effects
	Array<ModelEffect> Fx;

	// actions
	void SelectionAddEffects(const ModelEffect &effect);
	void EditEffect(int index, const ModelEffect &effect);
	void SelectionClearEffects();

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
	int ViewStage;

	bool ForceNewNormals[4];
	bool NormalIndicesOK[4];
};

#endif /* DATAMODEL_H_ */

/*
 * DataModel.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef DATAMODEL_H_
#define DATAMODEL_H_

#include "../Data.h"
#include "../../MultiView/SingleData.h"
#include "../../lib/base/set.h"
#include "../../x/model.h"
#include "../../x/material.h"
#include "ModelPolygon.h"
#include "ModelSurface.h"
#include "ModelMaterial.h"

class DataModel;
class ModelSurface;
class ModelMaterial;
class Geometry;


class GeometryException : public ActionException
{
public:
	GeometryException(const string &message) : ActionException(message){}
};

#define TransparencyModeDefault			-1
#define TransparencyModeNone			0
#define TransparencyModeFunctions		1
#define TransparencyModeColorKeyHard	2
#define TransparencyModeColorKeySmooth	3
#define TransparencyModeFactor			4

// effects
enum{
	FX_KIND_SCRIPT,
	FX_KIND_LIGHT,
	FX_KIND_FOG,
	FX_KIND_SOUND,
	FX_KIND_FORCEFIELD
};

struct ModelEffect
{
	int kind, surface, vertex;
	float size, speed, intensity;
	color colors[3];
	bool inv_quad;
	string file;

	void clear();
	string get_type();
};

class ModelVertex: public MultiView::SingleData
{
public:
	int normal_mode;
	int bone_index;

	bool normal_dirty;
	int ref_count; // polygons
	int surface;
};

// only for use in MultiView...
class ModelSkinVertexDummy: public MultiView::SingleData
{
};

class ModelBall: public MultiView::SingleData
{
public:
	int index;
	float radius;
};

struct ModelPolyhedronFace
{
	int NumVertices;
	int Index[MODEL_MAX_POLY_VERTICES_PER_FACE];
	plane Plane;
};


// TODO: dynamical!
class ModelPolyhedron: public MultiView::SingleData
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

class ModelTriangle: public MultiView::SingleData
{
public:
	int vertex[3];
	//int Edge[3];
	//int EdgeDirection[3]; // which no of triangle in edge's list are we?
	vector skin_vertex[MATERIAL_MAX_TEXTURES][3];
	int normal_index[3];
	vector normal[3];
	vector temp_normal;
	bool normal_dirty;
	int Material;
};


// triangles belonging to one material
struct ModelSubSkin
{
	int num_textures; // "read only" (updated automatically...)

	// triangles
	Array<ModelTriangle> triangle;
};


// geometry
struct ModelSkin
{
	// vertices
	Array<ModelVertex> vertex;

	// sub skins
	Array<ModelSubSkin> sub;
};

class ModelBone: public MultiView::SingleData
{
public:
	int parent;
	string model_file;
	Model *model;
	bool const_pos;

	// for editing
	matrix _matrix;
};

struct ModelFrame
{
	// skeleton animation
	Array<vector> skel_dpos;
	Array<vector> skel_ang;

	// vertex animation
	struct{
		Array<vector> dpos;
	}skin[4];
	Array<vector> vertex_dpos;
};

struct ModelMove
{
	int type;
	Array<ModelFrame> frame;
	float frames_per_sec_const, frames_per_sec_factor;
	bool interpolated_quadratic, interpolated_loop;
	string name;
};

class ModelEdge: public MultiView::SingleData
{
public:
	//int NormalMode;
	int vertex[2];
	int ref_count, polygon[2], side[2];
	bool is_round; // for editing
	float weight; // for easify'ing

	// constraints:
	//  Vertex[0] = surf.Polygon[Triangle[0]].Vertex[Side[0]]
	//  Vertex[1] = surf.Polygon[Triangle[0]].Vertex[(Side[0] + 1) % 3]
	//  same for Polygon/Side[1] but Vertex[0 <-> 1]

	virtual bool hover(MultiView::Window *win, vector &m, vector &tp, float &z, void *user_data);
	virtual bool inRect(MultiView::Window *win, rect &r, void *user_data);
};


class ModelSelectionState
{
public:
	struct EdgeSelection
	{
		EdgeSelection(){};
		EdgeSelection(int v[2]);
		int v[2];
	};
	Set<int> vertex;
	Set<int> surface;
	Array<Set<int> > polygon;
	Array<Array<EdgeSelection> > edge;
	void clear();
};


class DataModel: public Data
{
public:
	DataModel();
	virtual ~DataModel();

	static const string MESSAGE_SELECTION;
	static const string MESSAGE_SKIN_CHANGE;

	virtual void reset();
	virtual bool load(const string &_filename, bool deep = true);
	virtual bool save(const string &_filename);

	void ImportFromTriangleSkin(int index);
	void ExportToTriangleSkin(int index);

	void DebugShow();
	virtual bool testSanity(const string &loc);
	virtual void onPostActionUpdate();

	void SetNormalsDirtyByVertices(const Array<int> &index);
	void SetAllNormalsDirty();
	void UpdateNormals();



	int GetNumSelectedVertices();
	int GetNumSelectedSkinVertices();
	int GetNumSelectedPolygons();
	int GetNumSelectedSurfaces();
	//int GetNumSelectedBalls();
	int GetNumSelectedBones();
	int GetNumPolygons();

	void ClearSelection();
	void SelectionFromVertices();
	void SelectionFromPolygons();
	void SelectionFromEdges();
	void SelectionFromSurfaces();
	void GetSelectionState(ModelSelectionState &s);
	void SetSelectionState(ModelSelectionState &s);
	Set<int> GetSelectedSurfaces();
	Set<int> GetSelectedVertices();
	void SelectOnlySurface(ModelSurface *s);


	float GetRadius();
	void GetBoundingBox(vector &min, vector &max);
	void GenerateDetailDists(float *dist);
	matrix3 GenerateInertiaTensor(float mass);
	void CreateSkin(ModelSkin *src, ModelSkin *dst, float quality_factor);

	// low level (un-action'ed)
	//void LowLevelAddVertex(const vector &vd);
	ModelSurface *AddSurface(int surf_no = -1);
	ModelSurface *SurfaceJoin(ModelSurface *a, ModelSurface *b);
	int get_surf_no(ModelSurface *s);

	// high level (actions)
	void AddVertex(const vector &pos, int bone_index = 0, int normal_mode = -1);
	ModelPolygon *AddTriangle(int a, int b, int c, int material);
	ModelPolygon *AddPolygon(Array<int> &v, int material);
	ModelPolygon *AddPolygonWithSkin(Array<int> &v, Array<vector> &sv, int material);

	void DeleteSelection(bool greedy = false);
	void InvertSelection();
	void InvertSurfaces(const Set<int> &surfaces);
	void SubtractSelection();
	void AndSelection();
	void CutOutSelection();
	void ConvertSelectionToTriangles();
	void MergePolygonsSelection();
	void AlignToGridSelection(float grid_d);
	void NearifySelectedVertices();
	void CollapseSelectedVertices();
	void BevelSelectedEdges(float radius);
	void FlattenSelectedVertices();
	void TriangulateSelectedVertices();
	void ExtrudeSelectedPolygons(float offset);
	void AutoWeldSurfaces(const Set<int> &surfaces, float d);
	void AutoWeldSelectedSurfaces(float d);
	void SetNormalModeSelection(int mode);
	void SetMaterialSelection(int material);
	void CopyGeometry(Geometry &geo); // not an action...
	void PasteGeometry(Geometry &geo, int default_material);
	void Easify(float factor);
	void SubdivideSelectedSurfaces();
	void Automap(int material, int texture_level);


	// properties
	Array<ModelBone> bone;


	// properties
	Array<ModelMove> move;

	// actions
	void AddAnimation(int index, int type);
	void DeleteAnimation(int index);
	void AnimationAddFrame(int index, int frame);
	void AnimationDeleteFrame(int index, int frame);

	// geometry
	Array<ModelVertex> vertex;
	Array<ModelSurface> surface;
	Array<ModelSkinVertexDummy> skin_vertex; // only temporary...

	// old geometry
	ModelSkin skin[4];

	// geometry (physical)
	Array<ModelBall> ball;
	Array<ModelPolyhedron> poly;

	// general properties
	Array<ModelMaterial> material;
	vector _min, _max;

	// effects
	Array<ModelEffect> fx;

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



	float radius;




	// editor

	bool ForceNewNormals[4];
	bool NormalIndicesOK[4];
};

#endif /* DATAMODEL_H_ */

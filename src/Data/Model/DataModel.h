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


struct ModelEffect
{
	int type, surface, vertex;
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

	ModelVertex(){}
	ModelVertex(const vector &pos);
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

class ModelCylinder: public MultiView::SingleData
{
public:
	int index[2];
	float radius;
	bool round;
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
	float duration;

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

	bool needsRubberTiming();
	float duration();
	void getTimeInterpolation(float time, int &frame0, int &frame1, float &t);
	ModelFrame interpolate(float time);
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

class ModelScriptVariable
{
public:
	string name, type, value;
};

class DataModel: public Data
{
public:
	DataModel();
	virtual ~DataModel();

	static const string MESSAGE_SKIN_CHANGE;
	static const string MESSAGE_MATERIAL_CHANGE;
	static const string MESSAGE_TEXTURE_CHANGE;

	void reset() override;

	void importFromTriangleSkin(int index);
	void exportToTriangleSkin(int index);

	void debugShow();
	virtual bool test_sanity(const string &loc);
	virtual void on_post_action_update();

	void setNormalsDirtyByVertices(const Array<int> &index);
	void setAllNormalsDirty();
	void updateNormals();



	int getNumSelectedVertices();
	int getNumSelectedSkinVertices();
	int getNumSelectedEdges();
	int getNumSelectedPolygons();
	int getNumSelectedSurfaces();
	//int GetNumSelectedBalls();
	int getNumSelectedBones();
	int getNumPolygons();

	void clearSelection();
	void selectionFromVertices();
	void selectionFromPolygons();
	void selectionFromEdges();
	void selectionFromSurfaces();
	void getSelectionState(ModelSelectionState &s);
	void setSelectionState(ModelSelectionState &s);
	Set<int> getSelectedSurfaces();
	Set<int> getSelectedVertices();
	void selectOnlySurface(ModelSurface *s);


	float getRadius();
	void getBoundingBox(vector &min, vector &max);
	void generateDetailDists(float *dist);
	matrix3 generateInertiaTensor(float mass);
	void createSkin(ModelSkin *src, ModelSkin *dst, float quality_factor);

	// low level (un-action'ed)
	//void LowLevelAddVertex(const vector &vd);
	ModelSurface *addSurface(int surf_no = -1);
	ModelSurface *surfaceJoin(ModelSurface *a, ModelSurface *b);
	int get_surf_no(ModelSurface *s);

	// high level (actions)
	void addVertex(const vector &pos, int bone_index = 0, int normal_mode = -1);
	ModelPolygon *addTriangle(int a, int b, int c, int material);
	ModelPolygon *addPolygon(Array<int> &v, int material);
	ModelPolygon *addPolygonWithSkin(Array<int> &v, Array<vector> &sv, int material);

	void deleteSelection(bool greedy = false);
	void invertSelection();
	void invertSurfaces(const Set<int> &surfaces);
	void subtractSelection();
	void andSelection();
	void cutOutSelection();
	void convertSelectionToTriangles();
	void mergePolygonsSelection();
	void alignToGridSelection(float grid_d);
	void nearifySelectedVertices();
	void collapseSelectedVertices();
	void bevelSelectedEdges(float radius);
	void flattenSelectedVertices();
	void triangulateSelectedVertices();
	void extrudeSelectedPolygons(float offset, bool independent);
	void autoWeldSurfaces(const Set<int> &surfaces, float d);
	void autoWeldSelectedSurfaces(float d);
	void setNormalModeSelection(int mode);
	void setMaterialSelection(int material);
	void copyGeometry(Geometry &geo); // not an action...
	void pasteGeometry(Geometry &geo, int default_material);
	void easify(float factor);
	void subdivideSelectedSurfaces();
	void automap(int material, int texture_level);


	// properties
	Array<ModelBone> bone;

	// actions
	void reconnectBone(int index, int parent);
	void setBoneModel(int index, const string &filename);
	void addBone(const vector &pos, int parent);
	void deleteBone(int index);
	void deleteSelectedBones();
	void boneAttachVertices(int index, const Array<int> &vertices);


	// properties
	Array<ModelMove> move;

	// actions
	void addAnimation(int index, int type);
	void duplicateAnimation(int source, int target);
	void deleteAnimation(int index);
	void setAnimationData(int index, const string &name, float fps_const, float fps_factor);
	void animationAddFrame(int index, int frame, const ModelFrame &f);
	void animationDeleteFrame(int index, int frame);
	void animationSetFrameDuration(int index, int frame, float duration);
	void animationSetBone(int move, int frame, int bone, const vector &dpos, const vector &ang);

	// geometry
	Array<ModelVertex> vertex;
	Array<ModelSurface> surface;
	Array<ModelSkinVertexDummy> skin_vertex; // only temporary...

	// old geometry
	ModelSkin skin[4];

	// geometry (physical)
	Array<ModelBall> ball;
	Array<ModelCylinder> cylinder;
	Array<ModelPolyhedron> poly;

	// general properties
	Array<ModelMaterial*> material;
	vector _min, _max;

	// effects
	Array<ModelEffect> fx;

	// actions
	void selectionAddEffects(const ModelEffect &effect);
	void editEffect(int index, const ModelEffect &effect);
	void selectionClearEffects();

	struct MetaData
	{
		// level of detail
		float detail_dist[3];
		int detail_factor[3];
		bool auto_generate_dists, auto_generate_skin[3];

		// physics
		float mass;
		bool active_physics, passive_physics;
		bool auto_generate_tensor;
		matrix3 inertia_tensor;

		// object-properties
		string name, description;
		Array<string> inventary;

		// script
		string script_file;
		Array<ModelScriptVariable> variables;
		Array<float> script_var;

		void reset();
	};
	MetaData meta_data;



	float radius;




	// editor

	bool force_new_normals[4];
	bool normal_indices_ok[4];


	void showVertices(Array<ModelVertex> &vert);
	Array<ModelVertex> show_vertices;
};

#endif /* DATAMODEL_H_ */

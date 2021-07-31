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
#include "../../y/Model.h"
#include "../../y/Material.h"
#include "ModelMaterial.h"

class DataModel;
class ModelMaterial;
class ModelMesh;
class ModelPolygon;
class ModelVertex;
class ModelEdge;
class Geometry;
class ModelSelection;


class GeometryException : public ActionException {
public:
	GeometryException(const string &message) : ActionException(message){}
};


struct ModelEffect {
	int type, vertex;
	float size, speed, intensity;
	color colors[3];
	bool inv_quad;
	Path file;

	void clear();
	string get_type();
};


class ModelTriangle: public MultiView::SingleData {
public:
	int vertex[3];
	//int Edge[3];
	//int EdgeDirection[3]; // which no of triangle in edge's list are we?
	vector skin_vertex[MATERIAL_MAX_TEXTURES][3];
	vector normal[3];
	vector temp_normal;
	bool normal_dirty;
	int material;
};


// triangles belonging to one material
struct ModelTriangleSubMesh {
	int num_textures; // "read only" (updated automatically...)

	// triangles
	Array<ModelTriangle> triangle;
};


// exported geometry
struct ModelTriangleMesh {
	// vertices
	Array<ModelVertex> vertex;

	// sub skins
	Array<ModelTriangleSubMesh> sub;
};

class ModelBone: public MultiView::SingleData {
public:
	int parent;
	Path model_file;
	Model *model;
	bool const_pos;

	// for editing
	matrix _matrix;
};

struct ModelFrame {
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

struct ModelMove {
	AnimationType type;
	int id;
	Array<ModelFrame> frame;
	float frames_per_sec_const, frames_per_sec_factor;
	bool interpolated_quadratic, interpolated_loop;
	string name;

	bool needsRubberTiming();
	float duration();
	void getTimeInterpolation(float time, int &frame0, int &frame1, float &t);
	ModelFrame interpolate(float time);
};



class ModelScriptVariable {
public:
	string name, type, value;
};

class DataModel: public Data {
public:
	DataModel();
	virtual ~DataModel();

	static const string MESSAGE_SKIN_CHANGE;
	static const string MESSAGE_MATERIAL_CHANGE;
	static const string MESSAGE_TEXTURE_CHANGE;

	void reset() override;

	void import_from_triangle_mesh(int index);

	void debug_show();
	bool test_sanity(const string &loc) override;
	void on_post_action_update() override;

	void set_normals_dirty_by_vertices(const Array<int> &index);
	void set_all_normals_dirty();
	void update_normals();



	int getNumSelectedSkinVertices();

	void clearSelection();
	void selectionFromVertices();
	void selectionFromPolygons();
	void selectionFromEdges();
	ModelSelection get_selection() const;
	void set_selection(const ModelSelection &s);


	float getRadius();
	void getBoundingBox(vector &min, vector &max);
	void generateDetailDists(float *dist);
	matrix3 generateInertiaTensor(float mass);
	void create_triangle_mesh(ModelTriangleMesh *src, ModelTriangleMesh *dst, float quality_factor);

	// high level (actions)
	void addVertex(const vector &pos, const ivec4 &bone_index = {0,0,0,0}, const vec4 &bone_weight = {1,0,0,0}, int normal_mode = -1);
	ModelPolygon *addTriangle(int a, int b, int c, int material);
	ModelPolygon *addPolygon(const Array<int> &v, int material);
	ModelPolygon *addPolygonWithSkin(const Array<int> &v, const Array<vector> &sv, int material);

	void delete_polygon(int index);

	void delete_selection(const ModelSelection &s, bool greedy = false);
	void invert_polygons(const ModelSelection &s);
	void subtractSelection(int view_stage);
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
	void pasteGeometry(Geometry &geo, int default_material);
	void easify(float factor);
	void subdivideSelectedSurfaces(const ModelSelection &s);
	void automap(int material, int texture_level);


	// properties
	Array<ModelBone> bone;

	// actions
	void reconnectBone(int index, int parent);
	void setBoneModel(int index, const Path &filename);
	void addBone(const vector &pos, int parent);
	void deleteBone(int index);
	void deleteSelectedBones();
	void boneAttachVertices(const Array<int> &vertices, const Array<ivec4> &bone, const Array<vec4> &weight);


	// properties
	Array<ModelMove> move;

	// actions
	void addAnimation(int index, AnimationType type);
	void duplicateAnimation(int source, int target);
	void deleteAnimation(int index);
	void setAnimationData(int index, const string &name, float fps_const, float fps_factor);
	void animationAddFrame(int index, int frame, const ModelFrame &f);
	void animationDeleteFrame(int index, int frame);
	void animationSetFrameDuration(int index, int frame, float duration);
	void animationSetBone(int move, int frame, int bone, const vector &dpos, const vector &ang);

	// geometry
	ModelMesh *mesh;
	ModelMesh *phys_mesh;
	ModelMesh *edit_mesh;


	// old geometry
	Array<ModelTriangleMesh> triangle_mesh;

	// general properties
	Array<ModelMaterial*> material;

	// effects
	Array<ModelEffect> fx;

	// actions
	void selectionAddEffects(const ModelEffect &effect);
	void editEffect(int index, const ModelEffect &effect);
	void selectionClearEffects();

	struct MetaData {
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
		Array<Path> inventary;

		// script
		Path script_file;
		Array<ModelScriptVariable> variables;
		Array<float> script_var;
		string _script_class;

		void reset();
	};
	MetaData meta_data;




	// editor

	bool force_new_normals[4];
	bool normal_indices_ok[4];

};

#endif /* DATAMODEL_H_ */

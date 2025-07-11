/*
 * DataModel.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef DATAMODEL_H_
#define DATAMODEL_H_

#include <data/Data.h>
#include <view/SingleData.h>
#include <lib/base/set.h>
#include <y/world/Model.h>
#include <y/world/Material.h>
#include "ModelMaterial.h"

struct MeshEdit;
struct Box;
class DataModel;
class ModelMaterial;
struct ModelMesh;
struct Polygon;
struct MeshVertex;
struct PolygonMesh;
class ModelSelection;
enum class AnimationType;


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


class ModelTriangle: public multiview::SingleData {
public:
	int vertex[3];
	//int Edge[3];
	//int EdgeDirection[3]; // which no of triangle in edge's list are we?
	vec3 skin_vertex[MATERIAL_MAX_TEXTURES][3];
	vec3 normal[3];
	vec3 temp_normal;
	bool normal_dirty;
	int material;
};


// triangles belonging to one material
struct ModelTriangleSubMesh {
	int num_textures; // "read only" (updated automatically...)

	// triangles
	Array<ModelTriangle> triangles;
};


// exported geometry
struct ModelTriangleMesh {
	// vertices
	Array<MeshVertex> vertices;

	// sub skins
	Array<ModelTriangleSubMesh> sub;
};

struct ModelBone: multiview::SingleData {
	int parent;
	Path model_file;
	Model *model;
	bool const_pos;

	// for editing
	mat4 _matrix;
};

struct ModelFrame {
	float duration;

	// skeleton animation
	Array<vec3> skel_dpos;
	Array<vec3> skel_ang;

	// vertex animation
	struct{
		Array<vec3> dpos;
	}skin[4];
	Array<vec3> vertex_dpos;
};

struct ModelMove {
	AnimationType type;
	int id;
	Array<ModelFrame> frames;
	float frames_per_sec_const, frames_per_sec_factor;
	bool interpolated_quadratic, interpolated_loop;
	string name;

	bool needsRubberTiming();
	float duration();
	void getTimeInterpolation(float time, int &frame0, int &frame1, float &t);
	ModelFrame interpolate(float time);
};



struct ModelScriptVariable {
	string name, type, value;
};

class DataModel: public Data {
public:
	explicit DataModel(Session* s);
	~DataModel() override;

	obs::source out_skin_changed{this, "skin-changed"};
	obs::source out_material_changed{this, "material-changed"};
	obs::source out_texture_changed{this, "texture-changed"};
	obs::source out_topology_changed{this, "topology-changed"};

	void reset() override;

	void import_from_triangle_mesh(int index);

	void debug_show();
	//bool test_sanity(const string &loc) override;
	//void on_post_action_update() override;

	//void set_normals_dirty_by_vertices(const Array<int> &index);
	//void set_all_normals_dirty();
	//void update_normals();



	//int getNumSelectedSkinVertices();

/*	void clearSelection();
	void selectionFromVertices();
	void selectionFromPolygons();
	void selectionFromEdges();
	ModelSelection get_selection() const;
	void set_selection(const ModelSelection &s);*/


	float get_radius();
	Box bounding_box();
	void generateDetailDists(float *dist);
	mat3 generateInertiaTensor(float mass);
	void create_triangle_mesh(ModelTriangleMesh *src, ModelTriangleMesh *dst, float quality_factor);

	// high level (actions)
	void add_vertex(const vec3 &pos, const ivec4 &bone_index = {0,0,0,0}, const vec4 &bone_weight = {1,0,0,0}, int normal_mode = -1);
	Polygon *add_triangle(int a, int b, int c, int material);
	Polygon *add_polygon(const Array<int> &v, int material);
	Polygon *add_polygon_with_skin(const Array<int> &v, const Array<vec3> &sv, int material);

	void edit_mesh(const MeshEdit& edit);

/*	void delete_polygon(int index);

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
	void autoWeldSurfaces(const base::set<int> &surfaces, float d);
	void autoWeldSelectedSurfaces(float d);
	void setNormalModeSelection(int mode);*/
	void apply_material(const Selection& sel, int material);
	/*void easify(float factor);
	void subdivideSelectedSurfaces(const ModelSelection &s);
	void automap(int material, int texture_level);*/

//	Selection get_selection() const override;

	// actions
	void delete_selection(const Selection& sel, bool greedy);
	void paste_mesh(const PolygonMesh& geo, int default_material = -1);


	// properties
	Array<ModelBone> bones;

	// actions
	/*void reconnectBone(int index, int parent);
	void setBoneModel(int index, const Path &filename);
	void addBone(const vec3 &pos, int parent);
	void deleteBone(int index);
	void deleteSelectedBones();
	void boneAttachVertices(const Array<int> &vertices, const Array<ivec4> &bone, const Array<vec4> &weight);*/


	// properties
	Array<ModelMove> moves;

	// actions
	/*void addAnimation(int index, AnimationType type);
	void duplicateAnimation(int source, int target);
	void deleteAnimation(int index);
	void setAnimationData(int index, const string &name, float fps_const, float fps_factor);
	void animationAddFrame(int index, int frame, const ModelFrame &f);
	void animationDeleteFrame(int index, int frame);
	void animationSetFrameDuration(int index, int frame, float duration);
	void animationSetBone(int move, int frame, int bone, const vec3 &dpos, const vec3 &ang);*/

	// geometry
	owned<ModelMesh> mesh;
	owned<ModelMesh> phys_mesh;
	ModelMesh* editing_mesh;


	// old geometry
	Array<ModelTriangleMesh> triangle_mesh;

	// general properties
	Array<ModelMaterial*> materials;

	// effects
	Array<ModelEffect> fx;

	// actions
	/*void selectionAddEffects(const ModelEffect &effect);
	void editEffect(int index, const ModelEffect &effect);
	void selectionClearEffects();*/

	struct MetaData {
		// level of detail
		float detail_dist[3];
		int detail_factor[3];
		bool auto_generate_dists, auto_generate_skin[3];

		// physics
		float mass;
		bool active_physics, passive_physics;
		bool auto_generate_tensor;
		mat3 inertia_tensor;

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

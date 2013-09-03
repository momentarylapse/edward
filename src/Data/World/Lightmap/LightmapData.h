/*
 * LightmapData.h
 *
 *  Created on: 18.05.2013
 *      Author: michi
 */

#ifndef LIGHTMAPDATA_H_
#define LIGHTMAPDATA_H_

#include "../../../lib/base/base.h"
#include "../../../lib/math/math.h"

class DataModel;
class DataWorld;


class LightmapData
{
public:
	LightmapData(DataWorld *w);
	virtual ~LightmapData();

	float emissive_brightness;
	bool allow_sun;
	bool replace_objects;

	string world_name_small;
	string model_out_dir;
	string texture_out_dir;
	float color_exponent;
	vector min, max, center;
	float large_distance;
	float area;

	float resolution;

	void Init(DataWorld *w);
	void AddModel(const string &filename, matrix &mat, int object_index);
	void AddTextureLevels(bool modify = true);
	void CreateVertices();

	void SetResolution(float res);
	float GuessResolution();

	struct Model
	{
		int id;
		DataModel *orig;
		matrix mat;
		int offset, num_trias;
		string orig_name;
		string new_name;
		int object_index;
		float area;
		string tex_name;
		int tex_width, tex_height;
	};

	Array<Model> Models;

	// a "real" polygon in the models
	struct Triangle
	{
		vector v[3], sv[3], n[3], m;
		Ray ray[3];
		float r;
		/*bool inv;
		int permutation;*/
		plane pl;
		/*int nb, nc;
		int x, y;
		bool t_rot;*/
		int mod_id, surf, poly, side;
		color am, di, em;
		float area;
		int num_vertices;
		bool intersect(const Ray &r, vector &cp) const;
	};
	Array<Triangle> Trias;

	// a small sub triangle...
	struct Vertex
	{
		int x, y;
		vector pos, n;
		float area;
		color rad, _rad2, am, dif, em;
		int tria_id, mod_id;
		Array<int> visible;
		Array<float> coeff;
	};
	Array<Vertex> Vertices;

	bool IsVisible(const vector &a, const vector &b, int ignore_tria1, int ignore_tria2);
	bool IsVisible(Vertex &a, Vertex &b);


	struct Light
	{
		bool Directional;
		float Radius;
		vector Pos, Dir;
		color Ambient, Diffuse, Specular;
		//Array<int> visible[light_num_diffuse];
	};
	Array<Light> Lights;
	color Ambient;
};

#endif /* LIGHTMAPDATA_H_ */

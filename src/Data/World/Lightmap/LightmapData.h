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
	float color_exponent;
	vector min, max, center;
	float large_distance;

	void Init(DataWorld *w);
	void AddModel(const string &filename, matrix &mat, int object_index);

	struct Model
	{
		int id;
		DataModel *orig;
		int offset, num_trias;
		string tex_name;
		string orig_name;
		string new_name;
		int object_index;
	};

	Array<Model> Models;

	// a "real" polygon in the models
	struct Triangle
	{
		vector v[3], n[3], m;
		float r;
		bool inv;
		int permutation;
		plane pl;
		int nb, nc;
		int x, y;
		bool t_rot;
		int ob_id;
		color em;
	};
	Array<Triangle> Trias;


	struct Light
	{
		bool Directional;
		float Radius;
		vector Pos, Dir;
		color Ambient, Diffuse, Specular;
		//Array<int> visible[light_num_diffuse];
	};
	Array<Light> Lights;
};

#endif /* LIGHTMAPDATA_H_ */

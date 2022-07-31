/*
 * ActionModelAutomap.cpp
 *
 *  Created on: 12.05.2013
 *      Author: michi
 */

#include "ActionModelAutomap.h"
#include "../../../../lib/base/set.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../Data/Model/ModelMesh.h"
#include "../../../../Data/Model/ModelPolygon.h"

ActionModelAutomap::ActionModelAutomap(int _material, int _texture_level) {
	material = _material;
	texture_level = _texture_level;
}

struct Island {
	Set<int> p;
	vec3 dir;
	rect r;
	Array<vec3> skin;
	void map_primitive(DataModel *m);
	void apply(DataModel *m, int texture_level);
};

int max_index(Array<float> &d)
{
	int i_max = 0;
	float d_max = d[0];
	for (int i=1;i<d.num;i++)
		if (d[i] > d_max){
			d_max = d[i];
			i_max = i;
		}
	return i_max;
}

Array<int> group_by_dirs(DataModel *m, const vec3 *dir, int num_dirs)
{
	Array<int> r;
	foreachi(ModelPolygon &p, m->mesh->polygon, i){
		Array<float> d;
		for (int k=0;k<num_dirs;k++)
			d.add(vec3::dot(p.temp_normal, dir[k]));
		r.add(max_index(d));
	}
	return r;
}

Set<int> extract_connected(Set<int> &set, DataModel *m)
{
	Set<int> r;
	int i = set[0];
	r.add(i);
	set.erase(i);
	bool found = true;
	while(found){
		found = false;
		for (int i: r){
			ModelPolygon &p = m->mesh->polygon[i];
			for (int k=0;k<p.side.num;k++){
				int neigh = m->mesh->edge[p.side[k].edge].polygon[1 - p.side[k].edge_direction];
				if (neigh < 0)
					continue;
				if (set.find(neigh) >= 0){
					r.add(neigh);
					set.erase(neigh);
					found = true;
					break;
				}
				if (found)
					break;
			}
			if (found)
				break;
		}
	}
	return r;
}

Array<Island> find_connected(Set<int> &set, DataModel *m, const vec3 &dir)
{
	Array<Island> islands;
	while(set.num > 0){
		Island new_island;
		new_island.p = extract_connected(set, m);
		new_island.dir = dir;
		islands.add(new_island);
	}
	return islands;
}

Array<Island> get_islands(DataModel *m)
{
	const int num_dirs = 6;
	vec3 dir[num_dirs] = {vec3::EX, vec3::EY, vec3::EZ, -vec3::EX, -vec3::EY, -vec3::EZ};
	Array<Island> islands;
	Array<int> g = group_by_dirs(m, dir, num_dirs);
	for (int k=0;k<num_dirs;k++){
		Set<int> set;
		foreachi(int gg, g, i)
			if (gg == k)
				set.add(i);
		if (set.num == 0)
			continue;
		Array<Island> new_islands = find_connected(set, m, dir[k]);
		islands.append(new_islands);
	}
	return islands;
}

void Island::map_primitive(DataModel *m)
{
	vec3 e1 = dir.ortho();
	vec3 e2 = vec3::cross(dir, e1);
	skin.clear();

	// map (project on plane)
	for (int i: p){
		ModelPolygon &pp = m->mesh->polygon[i];
		for (int k=0;k<pp.side.num;k++){
			vec3 v = m->mesh->vertex[pp.side[k].vertex].pos;
			vec3 t = vec3(vec3::dot(v, e1), vec3::dot(v, e2), 0);
			skin.add(t);
		}
	}

	// find rotation with smallest width
	float phi_min = -1;
	float w_min = 0;
	for (float phi=0; phi<pi; phi += 0.05f){
		vec3 v = vec3(cos(phi), sin(phi), 0);
		float p_min = vec3::dot(v, skin[0]);
		float p_max = vec3::dot(v, skin[0]);
		for (int i=1; i<skin.num;i++){
			float p = vec3::dot(v, skin[i]);
			p_min = min(p_min, p);
			p_max = max(p_max, p);
		}
		float w = p_max - p_min;
		if ((w < w_min) || (phi == 0)){
			w_min = w;
			phi_min = phi;
		}
	}

	// rotate
	mat4 rot;
	rot = mat4::rotation_z( -phi_min);
	for (vec3 &v: skin)
		v = rot * v;


	// find boundary box
	r = rect(skin[0].x, skin[0].x, skin[0].y, skin[0].y);
	for (vec3 &v: skin){
		r.x1 = min(r.x1, v.x);
		r.x2 = max(r.x2, v.x);
		r.y1 = min(r.y1, v.y);
		r.y2 = max(r.y2, v.y);
	}

	// shift boundary to origin
	for (vec3 &v: skin)
		v -= vec3(r.x1, r.y1, 0);
	r.x2 -= r.x1;
	r.y2 -= r.y1;
	r.x1 = r.y1 = 0;
}

void Island::apply(DataModel *m, int texture_level)
{
	int n = 0;
	for (int i: p){
		ModelPolygon &pp = m->mesh->polygon[i];
		for (int k=0;k<pp.side.num;k++)
			pp.side[k].skin_vertex[texture_level] = skin[n ++];
	}
}

void optimize_islands(Array<Island> &islands, float distance)
{
	// guess texture size
	float sum_area = 0;
	float w_max = 0;
	for (Island &i: islands){
		w_max = max(w_max, i.r.width());
		sum_area += i.r.area();
	}
	float w = max(w_max, sqrt(sum_area * 1.5f));
	distance *= w;

	// sort by height
	for (int i=0;i<islands.num;i++)
		for (int j=i+1;j<islands.num;j++)
			if (islands[i].r.height() < islands[j].r.height())
				islands.swap(i, j);

	// map
	float x = 0, y = 0;
	float h = 0;
	w_max = 0;
	for (Island &i: islands){
		if (x + i.r.width() > w){
			x = 0;
			y += h + distance;
			h = 0;
		}
		i.r.x1 += x;
		i.r.x2 += x;
		i.r.y1 += y;
		i.r.y2 += y;
		x += i.r.width() + distance;
		w_max = max(w_max , x);
		h = max(h, i.r.height());
	}
	y += h + distance;

	// normalize
	for (Island &i: islands){
		for (vec3 &v: i.skin){
			v += vec3(i.r.x1, i.r.y1, 0);
			v.x /= w_max;
			v.y /= y;
		}
	}
}

void *ActionModelAutomap::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	// save old
	old_pos.clear();
	for (ModelPolygon &p: m->mesh->polygon)
		for (int k=0;k<p.side.num;k++)
			old_pos.add(p.side[k].skin_vertex[texture_level]);

	Array<Island> islands = get_islands(m);
	for (Island &i: islands)
		i.map_primitive(m);

	optimize_islands(islands, 0.01f);

	for (Island &i: islands)
		i.apply(m, texture_level);

	return NULL;
}

const string &ActionModelAutomap::message()
{
	return DataModel::MESSAGE_SKIN_CHANGE;
}

void ActionModelAutomap::undo(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);

	int n = 0;
	for (ModelPolygon &p: m->mesh->polygon)
		for (int k=0;k<p.side.num;k++)
			p.side[k].skin_vertex[texture_level] = old_pos[n ++];
}


/*
 * ActionModelAutomap.cpp
 *
 *  Created on: 12.05.2013
 *      Author: michi
 */

#include "ActionModelAutomap.h"
#include "../../../../lib/base/set.h"
#include "../../../../Data/Model/DataModel.h"

ActionModelAutomap::ActionModelAutomap(int _material, int _texture_level)
{
	material = _material;
	texture_level = _texture_level;
}

ActionModelAutomap::~ActionModelAutomap()
{
}

struct Island
{
	int surf;
	Set<int> p;
	vector dir;
	rect r;
	Array<vector> skin;
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

Array<int> group_by_dirs(ModelSurface *s, const vector *dir, int num_dirs)
{
	Array<int> r;
	foreachi(ModelPolygon &p, s->polygon, i){
		Array<float> d;
		for (int k=0;k<num_dirs;k++)
			d.add(p.temp_normal * dir[k]);
		r.add(max_index(d));
	}
	return r;
}

Set<int> extract_connected(Set<int> &set, ModelSurface *s)
{
	Set<int> r;
	int i = set[0];
	r.add(i);
	set.erase(i);
	bool found = true;
	while(found){
		found = false;
		for (int i: r){
			ModelPolygon &p = s->polygon[i];
			for (int k=0;k<p.side.num;k++){
				int neigh = s->edge[p.side[k].edge].polygon[1 - p.side[k].edge_direction];
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

Array<Island> find_connected(Set<int> &set, int surf, ModelSurface *s, const vector &dir)
{
	Array<Island> islands;
	while(set.num > 0){
		Island new_island;
		new_island.surf = surf;
		new_island.p = extract_connected(set, s);
		new_island.dir = dir;
		islands.add(new_island);
	}
	return islands;
}

Array<Island> get_islands(DataModel *m)
{
	const int num_dirs = 6;
	vector dir[num_dirs] = {e_x, e_y, e_z, -e_x, -e_y, -e_z};
	Array<Island> islands;
	foreachi(ModelSurface &s, m->surface, surf){
		Array<int> g = group_by_dirs(&s, dir, num_dirs);
		for (int k=0;k<num_dirs;k++){
			Set<int> set;
			foreachi(int gg, g, i)
				if (gg == k)
					set.add(i);
			if (set.num == 0)
				continue;
			Array<Island> new_islands = find_connected(set, surf, &s, dir[k]);
			islands.append(new_islands);
		}
	}
	return islands;
}

void Island::map_primitive(DataModel *m)
{
	vector e1 = dir.ortho();
	vector e2 = dir ^ e1;
	ModelSurface *s = &m->surface[surf];
	skin.clear();

	// map (project on plane)
	for (int i: p){
		ModelPolygon &pp = s->polygon[i];
		for (int k=0;k<pp.side.num;k++){
			vector v = m->vertex[pp.side[k].vertex].pos;
			vector t = vector(v * e1, v * e2, 0);
			skin.add(t);
		}
	}

	// find rotation with smallest width
	float phi_min = -1;
	float w_min;
	for (float phi=0; phi<pi; phi += 0.05f){
		vector v = vector(cos(phi), sin(phi), 0);
		float p_min = v * skin[0];
		float p_max = v * skin[0];
		for (int i=1; i<skin.num;i++){
			float p = v * skin[i];
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
	matrix rot;
	MatrixRotationZ(rot, -phi_min);
	for (vector &v: skin)
		v = rot * v;


	// find boundary box
	r = rect(skin[0].x, skin[0].x, skin[0].y, skin[0].y);
	for (vector &v: skin){
		r.x1 = min(r.x1, v.x);
		r.x2 = max(r.x2, v.x);
		r.y1 = min(r.y1, v.y);
		r.y2 = max(r.y2, v.y);
	}

	// shift boundary to origin
	for (vector &v: skin)
		v -= vector(r.x1, r.y1, 0);
	r.x2 -= r.x1;
	r.y2 -= r.y1;
	r.x1 = r.y1 = 0;
}

void Island::apply(DataModel *m, int texture_level)
{
	ModelSurface *s = &m->surface[surf];

	int n = 0;
	for (int i: p){
		ModelPolygon &pp = s->polygon[i];
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
		for (vector &v: i.skin){
			v += vector(i.r.x1, i.r.y1, 0);
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
	for (ModelSurface &s: m->surface)
		for (ModelPolygon &p: s.polygon)
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
	for (ModelSurface &s: m->surface)
		for (ModelPolygon &p: s.polygon)
			for (int k=0;k<p.side.num;k++)
				p.side[k].skin_vertex[texture_level] = old_pos[n ++];
}


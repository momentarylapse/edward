/*
 * LightmapPhotonMap.cpp
 *
 *  Created on: 19.05.2013
 *      Author: michi
 */

#include "LightmapPhotonMap.h"
#include "LightmapData.h"

LightmapPhotonMap::LightmapPhotonMap(LightmapData *_data) :
	Lightmap(_data)
{
}

LightmapPhotonMap::~LightmapPhotonMap()
{
}

void LightmapPhotonMap::Compute()
{
}


vector get_rand_dir(const vector &n)
{
	vector dir;
	do{
		dir.x = randf(2) - 1;
		dir.y = randf(2) - 1;
		dir.z = randf(2) - 1;
	}while (dir.length_sqr() > 1);
	if ((dir * n) < 0)
		dir = - dir;
	dir.normalize();
	return dir;
}

bool _LineIntersectsTriangle2_(const plane &pl,const vector &t1,const vector &t2,const vector &t3,const vector &l1,const vector &l2,vector &col,float &f, float &g)
{
	if (!pl.intersect_line(l1, l2, col))
		return false;
	GetBaryCentric(col,t1,t2,t3,f,g);
	if ((f>0)&&(g>0)&&(f+g<1))
		return true;
	return false;
}

void LightmapPhotonMap::Trace(Array<PhotonEvent> &ph, const vector &p, const vector &dir, const color &c, int ignore_tria, int n)
{
//	msg_db_f("PMTrace", 1);
	vector p2 = p + dir * data->large_distance;
	//printf("%f   %f   %f\n", dir.x, dir.y, dir.z);
	// trace
	int hit_tria = -1;
	vector hit_p;
	float f, g;
	for (int t=0;t<data->Trias.num;t++){
		if (t == ignore_tria)
			continue;
		LightmapData::Triangle &tria = data->Trias[t];
		vector cp;
		bool vm = false; // ???
/*		if (VecLineDistance(tria[t].m, p, p2) > tria[t].r)
		//if (_vec_line_distance_(tria[t].m, p1, p2) > tria[t].r)
			continue;*/

		// FIXME:  LineIntersectsTriangleF/G lokal!!!
		float ff, gg;
		if (_LineIntersectsTriangle2_(tria.pl, tria.v[0], tria.v[1], tria.v[2], p, p2, cp, ff, gg)){
			if (_vec_between_(cp, p, p2)){
				hit_tria = t;
				hit_p = cp;
				p2 = cp;
				f = ff;
				g = gg;
			}
		}
	}

	if (hit_tria < 0)
		return;

	PhotonEvent e;
	e.v = hit_p;
	e.dir = dir;
	e.c = c;
	e.tria = hit_tria;
	e.f = f;
	e.g = g;
	e.n = data->Trias[hit_tria].n[0] + (data->Trias[hit_tria].n[1] - data->Trias[hit_tria].n[0]) * f + (data->Trias[hit_tria].n[2] - data->Trias[hit_tria].n[0]) * g;
	e.n.normalize();
	ph.add(e);

	if (n >= MAX_PM_HITS)
		return;

	float u = randf(1);
	if (u < 0.5f){
		// reflect
		vector dir2 = get_rand_dir(data->Trias[hit_tria].pl.n);
		Trace(ph, hit_p, dir2, c, hit_tria, n + 1);
	}else{
		// absorb
	}
}


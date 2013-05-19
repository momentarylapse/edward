/*
 * LightmapPhotonMap.cpp
 *
 *  Created on: 19.05.2013
 *      Author: michi
 */

#include "LightmapPhotonMap.h"
#include "LightmapData.h"
#include "../../../lib/file/file.h"

LightmapPhotonMap::LightmapPhotonMap(LightmapData *_data, int _num_photons) :
	Lightmap(_data)
{
	num_photons = _num_photons;

	// distribute photons/energy
	e_all = 0;

	foreachi(LightmapData::Triangle &t, data->Trias, i){
		if (t.em.r + t.em.g + t.em.b > 0.01f){
			// emission * area
			float e = (t.em.r + t.em.g + t.em.b) / 3 * t.area;
			msg_write(f2s(e, 3));
			e *= data->emissive_brightness;
			tria_e.add(e);
			e_all += e;
			tria_i.add(i);
		}
	}
	e_per_photon = e_all / (float)num_photons;
}

LightmapPhotonMap::~LightmapPhotonMap()
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

void LightmapPhotonMap::Compute()
{
	int work_id = 0;
	for (int i=0;i<tria_i.num;i++){
		int n_photons = (int)((float) num_photons * tria_e[i] /* / WorkGetNumThreads()*/ / e_all);
		int ti = tria_i[i];
		LightmapData::Triangle &t = data->Trias[ti];
		msg_write("----");
		for (int j=0;j<n_photons;j++){
			float f, g;
			do{
				f = randf(1.0f);
				g = randf(1.0f);
			}while(f + g > 1);
			vector p = t.v[0] + (t.v[1] - t.v[0]) * f + (t.v[2] - t.v[0]) * g;

			vector dir = get_rand_dir(t.pl.n);
			color c = t.em * (3.0f / (t.em.r + t.em.g + t.em.b));
			//msg_write(format("%d / %d", j, n_photons));
			Trace(thread_photon[work_id], p, dir, c * e_per_photon, ti, 0);
			/*done ++;
			if ((done % 100) == 99)
				pm_num_done += 100;
			//	Progress("", (float)done / (float)num_photons);*/
		}
		msg_write(thread_photon[work_id].num);
	}
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


Lightmap::Histogram LightmapPhotonMap::GetHistogram()
{
	msg_write(thread_photon[0].num);
	Array<float> e;
	e.resize(data->Trias.num);
	foreach(PhotonEvent &ev, thread_photon[0]){
		e[ev.tria] += (ev.c.r + ev.c.g + ev.c.b) / 3.0f / data->Trias[ev.tria].area;
	}
	Lightmap::Histogram h;
	h.max = 0;
	foreach(float ee, e)
		h.max = max(h.max, ee);
	const int N = 128;
	h.f.resize(N);
	foreach(float ee, e)
		if (ee > 0)
			h.f[ee * N / h.max] += 1;
	h.normalize();
	return h;
}

/*
 * LightmapPhotonMap.cpp
 *
 *  Created on: 19.05.2013
 *      Author: michi
 */

#include "LightmapPhotonMap.h"
#include "LightmapData.h"
#include "../../../lib/file/file.h"
#include "../../../lib/image/image.h"
#include "../../Model/DataModel.h"
#include "../../../Stuff/Progress.h"
#include "../../../Edward.h"

static float d_max = 20.0f;

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
	photon.append(thread_photon[work_id]);
	thread_photon[work_id].clear();

	CreateBalancedTree();
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
	msg_write(photon.num);
	Array<float> e;
	e.resize(data->Trias.num);
	foreach(PhotonEvent &ev, photon){
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


inline void p_xchg(LightmapPhotonMap::PhotonEvent *&a, LightmapPhotonMap::PhotonEvent *&b)
{
	LightmapPhotonMap::PhotonEvent *temp = a;
	a = b;
	b = temp;
}

#define gc(v)		( (t->dir == 0) ? (v).x : ( (t->dir == 1) ? (v).y : (v).z) )


int pm_partition(LightmapPhotonMap::PhotonEvent** p, int l, int r)
{
	int i = l - 1, j = r;
	double v = p[r]->tree_c;
	for (;;){
		while(p[++ i]->tree_c < v);
		while(v < p[-- j]->tree_c)
			if (j == i)
				break;
		if (i >= j)
			break;
		p_xchg(p[i], p[j]);
	}
	p_xchg(p[i], p[r]);
	return i;
}

void pm_quick(LightmapPhotonMap::PhotonEvent** p, int l, int r, int med)
{
	int i = pm_partition(p, l, r);

	if (i < med)
		pm_quick(p, i + 1, r, med);
	else if (i > med)
		pm_quick(p, l, i -1, med);
}

/*void quicksort(sPhotonEvent** p, int l, int r)
{
	if (r <= l)
		return;
	int i = pm_partition(p, l, r);
	quicksort(p, l, i - 1);
	quicksort(p, i + 1, r);
}*/

void pm_balance(int b, LightmapPhotonMap::PhotonEvent** p, int pnum, LightmapPhotonMap::Branch *tree)
{
	//msg_write(string2("%d", pnum));
	LightmapPhotonMap::Branch *t = &tree[b];
	t->left = t->right = NULL;

	// bounding box
	t->min = p[0]->v;
	t->max = p[0]->v;

	if (pnum == 1){
		t->p = p[0];
		return;
	}

	for (int i=1;i<pnum;i++){
		t->min._min(p[i]->v);
		t->max._max(p[i]->v);
	}

	// plane
	vector d = t->max - t->min;
	t->dir = 0;
	if ((d.y > d.x) && (d.y > d.z))
		t->dir = 1;
	else if ((d.z > d.x) && (d.z > d.y))
		t->dir = 2;



	int med = pnum / 2;
	for (int i=0;i<pnum;i++)
		p[i]->tree_c = gc(p[i]->v);


	// divide....
	pm_quick(p, 0, pnum - 1, med);
	t->p = p[med];

	// cut list in two
	int pnum1 = med;
	int pnum2 = pnum - med - 1;

	// recursion
	if (pnum1 > 0){
		pm_balance(b * 2, p, pnum1, tree);
		t->left = &tree[b * 2];
	}
	if (pnum2 > 0){
		pm_balance(b * 2 + 1, &p[med + 1], pnum2, tree);
		t->right = &tree[b * 2 + 1];
	}
}

void LightmapPhotonMap::CreateBalancedTree()
{
	tree.resize(photon.num * 2);
	for (int i=0;i<tree.num;i++)
		tree[i].p = NULL;
	Array<PhotonEvent*> pp;
	pp.resize(photon.num);
	for (int i=0;i<photon.num;i++)
		photon[i].index = i;
	for (int i=0;i<photon.num;i++)
		pp[i] = &photon[i];
	pm_balance(1, &pp[0], photon.num, &tree[0]);
	pp.clear();
}


inline void pm_insert(LightmapPhotonMap::PhotonEvent *p, LightmapPhotonMap::PhotonEvent **l, int &lnum, int thread_id)
{
	for (int i=0;i<lnum;i++)
		if (p->tree_r2[thread_id] < l[i]->tree_r2[thread_id]){
			for (int j=lnum-1;j>=i;j--)
				l[j + 1] = l[j];
			l[i] = p;
			if (lnum < MAX_PHOTONS_PER_VERTEX)
				lnum ++;
			return;
		}
	if (lnum < MAX_PHOTONS_PER_VERTEX)
		l[lnum ++] = p;
}

int nsearch, nadd;
#if 1
void pm_tree_get_list(LightmapPhotonMap *lm, const vector &p, const vector &n, int b, LightmapPhotonMap::PhotonEvent **l, int &lnum, float &max_r2, int thread_id)
{
	if (b >= lm->tree.num)
		return;
	LightmapPhotonMap::Branch *t = &lm->tree[b];
	if (t->p){
		nsearch ++;
		float delta;
		if (t->dir == 0)
			delta = (p.x - t->p->v.x);
		else if (t->dir == 1)
			delta = (p.y - t->p->v.y);
		else
			delta = (p.z - t->p->v.z);
		//printf("%f\n", delta);

		if (delta < 0){
			pm_tree_get_list(lm, p, n, 2 * b, l, lnum, max_r2, thread_id);
			if (delta * delta < max_r2)
				pm_tree_get_list(lm, p, n, 2 * b + 1, l, lnum, max_r2, thread_id);
		}else{
			pm_tree_get_list(lm, p, n, 2 * b + 1, l, lnum, max_r2, thread_id);
			if (delta * delta < max_r2)
				pm_tree_get_list(lm, p, n, 2 * b, l, lnum, max_r2, thread_id);
		}

		if (t->p->n * n < 0)
			return;

		float d2 = (p - t->p->v).length_sqr();
		//printf("%f  %d\n", d2, b);
		if ((d2 < max_r2) && (VecDotProduct(n, t->p->n) > 0.7f)){
			//printf("----found!!! %f\n", d2);
			t->p->tree_r2[thread_id] = d2;
			pm_insert(t->p, l, lnum, thread_id);
			if (lnum >= MAX_PHOTONS_PER_VERTEX)
				max_r2 = l[lnum - 1]->tree_r2[thread_id];
		}
	}
}
#endif

void pm_get_list(LightmapPhotonMap *lm, const vector &p, const vector &n, LightmapPhotonMap::PhotonEvent **l, int &lnum, float max_r2, int thread_id)
{
	for (int i=0;i<lm->photon.num;i++){

		float d2 = (p - lm->photon[i].v).length_sqr();
		//printf("%f  %d\n", d2, b);
		if (d2 < max_r2){
			//printf("----found!!!\n");
			lm->photon[i].tree_r2[thread_id] = d2;
			pm_insert(&lm->photon[i], l, lnum, thread_id);
		}
	}
}

color RenderTreeVertex(int work_id, const vector &pos, const vector &n, LightmapPhotonMap *lm, LightmapData::Triangle &tria)
{
	//int t = vertex[v].tria_all;

	// emitting -> set color
	if (tria.em.r + tria.em.g + tria.em.b > 0.01f){
		return tria.em;
	}


	// hard cut off for search area
	float search_r2 = d_max * d_max * 50;

	// look up photons
	nsearch = 0;
	nadd = 0;
	LightmapPhotonMap::PhotonEvent* l[MAX_PHOTONS_PER_VERTEX + 10];
	int lnum = 0;
	pm_tree_get_list(lm, pos, n, 1, l, lnum, search_r2, work_id);
	//pm_get_list(lm, pos, n, l, lnum, search_r2, work_id);
	//printf("%d  %d\n", nsearch, lnum);

	/*sPhotonEvent* l2[MAX_PHOTONS_PER_VERTEX + 10];
	int lnum2 = 0;
	pm_get_list(vertex[v].v, vertex[v].n, l2, lnum2, search_r2, work_id);
	printf("%d  %d\n", nsearch, lnum2);

	if (lnum != lnum2){
		msg_error(string2(" %d != %d\n", lnum, lnum2));
	}else{
		for (int i=0;i<lnum;i++)
			if (l[i] != l2[i])
				msg_error(string2("%d:  %f != %f\n", i, l[i]->tree_r2[work_id], l2[i]->tree_r2[work_id]));
	}

	msg_write(string2("%d  %d", v, lnum));*/

	// mix colors
	color e = Black;
	if (lnum > 0){
		//printf("search %d  (%d) %d  %.1f  %f.1\n", nsearch, nadd, lnum, l[lnum - 1]->tree_r2, l[0]->tree_r2);
		float alpha = 0.918f;
		float beta = 1.953f;
		for (int j=0;j<lnum;j++){
			float r2 = l[j]->tree_r2[work_id] / l[lnum - 1]->tree_r2[work_id];
			float w = 1;//alpha * (1.0f - (1 - exp(-beta * r2 / 2)) / (1 - exp(-beta)));
			e += l[j]->c * w;
		}

		// area given by farthest photon
		float area = pi * l[lnum - 1]->tree_r2[work_id];
		//	printf("%f\n", area);

		// too few photons -> use complete search area
		if (lnum < MAX_PHOTONS_PER_VERTEX)
			area = pi * search_r2;

		e = e * (1.0f / area);
	}

	//e = (float)lnum / 1000.0f;
	//e /= 25000.0f;
	return SetColorSave(1, e.r, e.g, e.b);
}

rect get_tria_skin_boundary(vector sv[3])
{
	vector _min = sv[0];
	_min._min(sv[1]);
	_min._min(sv[2]);
	vector _max = sv[0];
	_max._max(sv[1]);
	_max._max(sv[2]);
	return rect(_min.x, _max.x, _min.y, _max.y);
}

void LightmapPhotonMap::RenderToTexture()
{
	foreach(LightmapData::Model &m, data->Models){
		int w = 512;
		int h = 512;
		Image im;
		im.Create(w, h, Black);

		for (int i=m.offset;i<m.offset + m.num_trias;i++){
			LightmapData::Triangle &t = data->Trias[i];
			ModelPolygon &p = m.orig->Surface[t.surf].Polygon[t.poly];
			vector v[3], sv[3];
			vector n = p.TempNormal;
			for (int k=0;k<3;k++){
				int si = p.Side[t.side].Triangulation[k];
				v[k] = m.orig->Vertex[p.Side[si].Vertex].pos;
				sv[k] = p.Side[si].SkinVertex[1];
				sv[k].x *= w;
				sv[k].y *= h;
			}
			rect r = get_tria_skin_boundary(sv);
			for (int x=r.x1-1;x<r.x2+1;x++)
				for (int y=r.y1-1;y<r.y2+1;y++){
					vector c = vector(x, y, 0);
					float f, g;
					GetBaryCentric(c, sv[0], sv[1], sv[2], f, g);
					if ((f >= 0) && (g >= 0) && (f + g <= 1)){
						vector pos = v[0] + f * (v[1] - v[0]) + g * (v[2] - v[0]);
						im.SetPixel(x, y, RenderTreeVertex(0, pos, n, this, t));
					}
				}
			ed->progress->Set(format(_("%d von %d"), i, data->Trias.num), (float)i / (float)data->Trias.num);
		}

		/*foreach(PhotonEvent &ev, photon){
			LightmapData::Triangle &t = data->Trias[ev.tria];
			ModelPolygon &p = m.orig->Surface[t.surf].Polygon[t.poly];
			vector sv[3];
			for (int k=0;k<3;k++)
				sv[k] = p.Side[p.Side[t.side].Triangulation[k]].SkinVertex[1];
			vector s = sv[0] + ev.f * (sv[1] - sv[0]) + ev.g * (sv[2] - sv[0]);
			ev.c.clamp();
			im.SetPixel(s.x * w, s.y * h, ev.c);
		}*/

		im.Save("new_lightmap.tga");
	}
}

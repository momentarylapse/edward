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
	total_energy = 0;

	foreachi(LightmapData::Triangle &t, data->Trias, i){
		if (t.em.r + t.em.g + t.em.b > 0.01f){
			// emission * area
			float e = (t.em.r + t.em.g + t.em.b) / 3 * t.area;
			e *= data->emissive_brightness;
			Emitter em;
			em.tria = i;
			em.energy = e;
			Emitters.add(em);
			total_energy += e;
		}
	}
	energy_per_photon = total_energy / (float)num_photons;
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
	int done = 0;
	foreach(Emitter &em, Emitters){
		int n_photons = (int)((float) num_photons * em.energy /* / WorkGetNumThreads()*/ / total_energy);
		LightmapData::Triangle &t = data->Trias[em.tria];
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
			Trace(thread_photon[work_id], p, dir, c * energy_per_photon, em.tria, 0);
			/*done ++;
			if ((done % 100) == 99)
				pm_num_done += 100;
			//	Progress("", (float)done / (float)num_photons);*/
			done ++;
			if ((done & 1023) == 0)
				ed->progress->Set(format(_("%d von %d"), done, num_photons), (float)done / (float)num_photons);
			if (ed->progress->IsCancelled())
				throw Lightmap::AbortException();
		}
	}
	photon.append(thread_photon[work_id]);
	thread_photon[work_id].clear();

	CreateBalancedTree();
}

void LightmapPhotonMap::Trace(Array<PhotonEvent> &ph, const vector &p, const vector &dir, const color &c, int ignore_tria, int n)
{
//	msg_db_f("PMTrace", 1);
	vector p2 = p + dir * data->large_distance;
	Ray r = Ray(p, p2);
	//printf("%f   %f   %f\n", dir.x, dir.y, dir.z);
	// trace
	int hit_tria = -1;
	vector hit_p;
	float f, g;
	for (int ti=0;ti<data->Trias.num;ti++){
		if (ti == ignore_tria)
			continue;
		LightmapData::Triangle &t = data->Trias[ti];
		vector cp;

		if (!t.intersect(r, cp))
			continue;
		if (!_vec_between_(cp, p, p2))
			continue;

		float ff, gg;
		GetBaryCentric(cp, t.v[0], t.v[1], t.v[2], ff, gg);

		hit_tria = ti;
		hit_p = cp;
		p2 = cp;
		f = ff;
		g = gg;
	}

	if (hit_tria < 0)
		return;

	LightmapData::Triangle &t = data->Trias[hit_tria];
	PhotonEvent e;
	e.pos = hit_p;
	e.dir = dir;
	e.c = c * data->Trias[hit_tria].di;
	e.tria = hit_tria;
	e.f = f;
	e.g = g;
	e.n = t.n[0] + (t.n[1] - t.n[0]) * f + (t.n[2] - t.n[0]) * g;
	e.n.normalize();
	ph.add(e);

	if (n >= MAX_PM_HITS)
		return;

	float u = randf(1);
	if (u < 0.5f){
		// reflect
		vector dir2 = get_rand_dir(t.pl.n);
		Trace(ph, hit_p, dir2, e.c, hit_tria, n + 1);
	}else{
		// absorb
	}
}


Lightmap::Histogram LightmapPhotonMap::GetHistogram()
{
	Array<float> e;
	e.resize(data->Trias.num);
	foreach(PhotonEvent &ev, photon)
		e[ev.tria] += (ev.c.r + ev.c.g + ev.c.b) / 3.0f / data->Trias[ev.tria].area;

	return Lightmap::Histogram(e);
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
	t->min = p[0]->pos;
	t->max = p[0]->pos;

	if (pnum == 1){
		t->p = p[0];
		return;
	}

	for (int i=1;i<pnum;i++){
		t->min._min(p[i]->pos);
		t->max._max(p[i]->pos);
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
		p[i]->tree_c = gc(p[i]->pos);


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


inline void pm_insert(LightmapPhotonMap::PhotonEvent *p, LightmapPhotonMap::PhotonEvent *l[], int &lnum, int thread_id)
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
void pm_tree_get_list(LightmapPhotonMap *lm, const vector &p, const vector &n, int b, LightmapPhotonMap::PhotonEvent *l[], int &lnum, float &max_r2, int thread_id)
{
	if (b >= lm->tree.num)
		return;
	LightmapPhotonMap::Branch *t = &lm->tree[b];
	if (t->p){
		nsearch ++;
		float delta;
		if (t->dir == 0)
			delta = (p.x - t->p->pos.x);
		else if (t->dir == 1)
			delta = (p.y - t->p->pos.y);
		else
			delta = (p.z - t->p->pos.z);
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

		float d2 = (p - t->p->pos).length_sqr();
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

void pm_get_list(LightmapPhotonMap *lm, const vector &p, const vector &n, LightmapPhotonMap::PhotonEvent *l[], int &lnum, float max_r2, int thread_id)
{
	for (int i=0;i<lm->photon.num;i++){

		float d2 = (p - lm->photon[i].pos).length_sqr();
		//printf("%f  %d\n", d2, b);
		if (d2 < max_r2){
			//printf("----found!!!\n");
			lm->photon[i].tree_r2[thread_id] = d2;
			pm_insert(&lm->photon[i], l, lnum, thread_id);
		}
	}
}

color LightmapPhotonMap::RenderVertex(LightmapData::Vertex &v)//(int work_id, const vector &pos, const vector &n, LightmapPhotonMap *lm, LightmapData::Triangle &tria)
{
	int work_id = 0;
	//int t = vertex[v].tria_all;

	// emitting -> set color
	if (v.em.r + v.em.g + v.em.b > 0.01f)
		return color(1, v.em.r, v.em.g, v.em.b);


	// hard cut off for search area
	float search_r2 = d_max * d_max * 50;

	// look up photons
	nsearch = 0;
	nadd = 0;
	LightmapPhotonMap::PhotonEvent* l[MAX_PHOTONS_PER_VERTEX + 10];
	int lnum = 0;
	pm_tree_get_list(this, v.pos, v.n, 1, l, lnum, search_r2, work_id);
	//pm_get_list(this, v.pos, v.n, l, lnum, search_r2, work_id);
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
	e.a = 1;
	e.clamp();
	if (data->color_exponent != 1){
		e.r = pow(e.r, data->color_exponent);
		e.g = pow(e.g, data->color_exponent);
		e.b = pow(e.b, data->color_exponent);
	}
	return e;
}

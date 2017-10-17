/*
 * LightmapPhotonMap.h
 *
 *  Created on: 19.05.2013
 *      Author: michi
 */

#ifndef LIGHTMAPPHOTONMAP_H_
#define LIGHTMAPPHOTONMAP_H_

#include "Lightmap.h"
#include "../../../lib/threads/Thread.h"
#include "../../../lib/threads/ThreadedWork.h"

#define MAX_PM_HITS			8

//#define MAX_PHOTONS_PER_VERTEX	1500
#define MAX_PHOTONS_PER_VERTEX	500


class LightmapPhotonMap : public Lightmap, public ThreadedWork
{
public:
	LightmapPhotonMap(LightmapData *data, int num_photons);
	virtual ~LightmapPhotonMap();

	virtual void Compute();
	virtual void _cdecl DoStep(int index, int worker_id);
	virtual bool _cdecl OnStatus();

	virtual void PrepareTextureRendering();
	virtual color RenderVertex(LightmapData::Vertex &v);
	virtual Histogram GetHistogram();

	void CreateBalancedTree();

	int num_photons;
	int done;
	int cur_em_tria;
	float total_energy, energy_per_photon;
	struct Emitter
	{
		int tria;
		float energy;
	};
	Array<Emitter> Emitters;


	struct PhotonEvent
	{
		vector pos, n, dir;
		color c; // power
		int tria;
		float f, g;

		float tree_r2[MAX_THREADS];
		double tree_c;

		int index;
	};

	Array<PhotonEvent> photon;
	Array<PhotonEvent> thread_photon[MAX_THREADS];
	Array<Array<PhotonEvent*> > list;

	struct Branch
	{
		PhotonEvent* p;
		vector min, max;
		int dir;
		Branch *left, *right;
	};
	Array<Branch> tree;

	void Trace(Array<PhotonEvent> &ph, const vector &pos, const vector &dir, const color &c, int ignore_tria, int n);
};

#endif /* LIGHTMAPPHOTONMAP_H_ */

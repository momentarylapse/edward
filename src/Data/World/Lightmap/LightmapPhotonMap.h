/*
 * LightmapPhotonMap.h
 *
 *  Created on: 19.05.2013
 *      Author: michi
 */

#ifndef LIGHTMAPPHOTONMAP_H_
#define LIGHTMAPPHOTONMAP_H_

#include "Lightmap.h"

#define MAX_PM_HITS			8

//#define MAX_PHOTONS_PER_VERTEX	1500
#define MAX_PHOTONS_PER_VERTEX	500

#define MAX_THREADS		8


class LightmapPhotonMap : public Lightmap
{
public:
	LightmapPhotonMap(LightmapData *data);
	virtual ~LightmapPhotonMap();

	virtual void Compute();


	struct PhotonEvent
	{
		vector v, n, dir;
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

	void Trace(Array<PhotonEvent> &ph, const vector &p, const vector &dir, const color &c, int ignore_tria, int n);
};

#endif /* LIGHTMAPPHOTONMAP_H_ */

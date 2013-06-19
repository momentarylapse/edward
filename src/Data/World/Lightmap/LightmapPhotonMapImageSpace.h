/*
 * LightmapPhotonMapImageSpace.h
 *
 *  Created on: 19.06.2013
 *      Author: michi
 */

#ifndef LIGHTMAPPHOTONMAPIMAGESPACE_H_
#define LIGHTMAPPHOTONMAPIMAGESPACE_H_

#include "LightmapPhotonMap.h"

class LightmapPhotonMapImageSpace : public LightmapPhotonMap
{
public:
	LightmapPhotonMapImageSpace(LightmapData *data, int num_photons);
	virtual ~LightmapPhotonMapImageSpace();


	void CreateTextures();
	virtual void PrepareTextureRendering();
	color get_color(LightmapData::Vertex &v, float r);
	virtual color RenderVertex(LightmapData::Vertex &v);


	struct PseudoImage
	{
		int w, h;
		Array<color> c;
	};

	Array<PseudoImage> psi;
};

#endif /* LIGHTMAPPHOTONMAPIMAGESPACE_H_ */

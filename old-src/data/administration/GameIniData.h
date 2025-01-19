/*
 * GameIniData.h
 *
 *  Created on: 02.03.2013
 *      Author: michi
 */

#ifndef GAMEINIDATA_H_
#define GAMEINIDATA_H_

#include "../Data.h"
#include "../../lib/os/config.h"


class GameIniData : public Configuration {
public:
	//Path default_script, default_world, second_world, default_material, default_font;
	void reset_default();
	void load(const Path &dir);
	void save(const Path &dir);

	Path default_script() const;
	Path default_world() const;
	Path second_world() const;
	Path default_material() const;
	Path default_font() const;

	static const string ID_SCRIPT;
	static const string ID_WORLD;
	static const string ID_WORLD2;
	static const string ID_MATERIAL;
	static const string ID_FONT;
	static const string ID_SCREEN_MODE;
	static const string ID_SCREEN_WIDTH;
	static const string ID_SCREEN_HEIGHT;
	static const string ID_RENDER_PATH;
	static const string ID_RENDERER_FRAMERATE;
	static const string ID_RESOLUTION_SCALE_MIN;
	static const string ID_SHADOW_BOXSIZE;
	static const string ID_SHADOW_RESOLUTION;
	static const string ID_DEBUG_LEVEL;
	static const string ID_DEBUG_SCRIPTS1;
	static const string ID_DEBUG_SCRIPTS2;
};


#endif /* GAMEINIDATA_H_ */

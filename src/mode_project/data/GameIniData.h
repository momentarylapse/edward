/*
 * GameIniData.h
 *
 *  Created on: 02.03.2013
 *      Author: michi
 */

#pragma once

#include <y/Config.h>


class GameIniData : public RawConfig {
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
};


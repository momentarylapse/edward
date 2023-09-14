/*
 * Config.h
 *
 *  Created on: 07.08.2020
 *      Author: michi
 */

#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_


#include <lib/base/base.h>
#include <lib/os/config.h>
#include <lib/os/path.h>

enum class AntialiasingMethod {
	NONE,
	MSAA,
	TAA
};

class Config : public Configuration {
public:
	int debug_level = 0;
	string main_script;
	string default_world;
	string second_world;
	string default_material;
	string default_font;
	Path game_dir;
	AntialiasingMethod antialiasing_method = AntialiasingMethod::NONE;
	bool allow_rtx = true;
	Array<string> additional_scripts;

	float resolution_scale_min = 0;
	float resolution_scale_max = 0;
	float target_framerate = 0;
	float ambient_occlusion_radius = 0;

	Config();
	void load(const Array<string> &arg);
};
extern Config config;

#endif /* SRC_CONFIG_H_ */

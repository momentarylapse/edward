/*
 * Config.cpp
 *
 *  Created on: 07.08.2020
 *      Author: michi
 */

#include "Config.h"
#include <lib/os/msg.h>
#include <lib/os/CommandLineParser.h>

Config config;

Config::Config() {
}

Array<float> parse_range(const string& a) {
	auto xx = a.explode(":");
	if (xx.num >= 2)
		return {xx[0]._float(), xx[1]._float()};
	return {xx[0]._float(), xx[0]._float()};
}

void Config::load(const Array<string> &arg) {
	// fixed config
	Configuration::load("game.ini");

	// local config override
	Configuration local;
	local.load("game-local.ini");
	for (auto&& [k, v]: local.map)
		set(k, v);

	// cli arguments override
	CommandLineParser p;
	p.info("y", "game engine");
	p.option("-c", "CONFIG", "set config A=B", [this] (const string& a) {
		auto xx = a.explode("=");
		set_str(xx[0].trim(), xx[1].trim());
	});
	p.option("-u/--uncapped", "uncapped framerate", [this] {
		set_bool("renderer.uncapped-framerate", true);
	});
	p.option("-D/--debug", "enable debug mode (level 2)", [this] {
		set_int("debug.level", 2);
	});
	p.option("-D0/--no-debug", "disable debug mode (level 0)", [this] {
		set_int("debug.level", 0);
	});
	p.option("--game-dir", "DIR", "set game directory", [this] (const string& a) {
		game_dir = a;
	});
	p.option("--fw/--forward", "use forward rendering path", [this] {
		set_str("renderer.path", "forward");
	});
	p.option("--def/--deferred", "use deferred rendering path", [this] {
		set_str("renderer.path", "deferred");
	});
	p.option("--direct", "use direct rendering path", [this] {
		set_str("renderer.path", "direct");
	});
	p.option("--rt/--raytracing", "use ray tracing rendering path", [this] {
		set_str("renderer.path", "raytracing");
	});
	p.option("--nortx", "don't use rtx even if available", [this] {
		allow_rtx = false;
	});
	p.option("--msaa", "use multi sampling anti aliasing", [this] {
		set_str("renderer.antialiasing", "MSAA");
	});
	p.option("--scale", "RANGE", "use resolutions scale MIN[:MAX]", [this] (const string& a) {
		auto r = parse_range(a);
		set_float("renderer.resolution-scale-min", r[0]);
		set_float("renderer.resolution-scale-max", r[1]);
	});
	p.option("--size", "SIZE", "set resolution WxH", [this] (const string& a) {
		auto xx = a.explode("x");
		set_int("screen.width", xx[0]._int());
		set_int("screen.height", xx[1]._int());
		set_str("screen.mode", "windowed");
	});
	p.option("--fps", "RANGE", "limit framerate in MIN[:MAX]", [this] (const string& a) {
		auto r = parse_range(a);
		set_float("renderer.min-framerate", r[0]);
		set_float("renderer.target-framerate", r[1]);
	});
	p.option("--script", "SCRIPT", "execute additional script", [this] (const string& a) {
		additional_scripts.add(a);
	});
	p.cmd("--help", "", "show help", [&p] (const Array<string>& a) {
		p.show();
		exit(0);
	});
	p.cmd("", "[WORLD]", "run game (optionally select first world)", [this, &p] (const Array<string>& a) {
		if (a.num > 0)
			set_str("default.world", a[0]);
	});
	p.parse(arg);

	// deprecated
	if (has("default-world") and !has("default.world"))
		set_str("default.world", get_str("default-world", ""));
	if (has("second-world") and !has("default.second-world"))
		set_str("default.second-world", get_str("second-world", ""));
	if (has("default-font") and !has("default.font"))
		set_str("default.font", get_str("default-font", ""));
	if (has("default-material") and !has("default.material"))
		set_str("default.material", get_str("default-material", ""));
	if (has("main-script") and !has("default.main-script"))
		set_str("default.main-script", get_str("main-script", ""));

	default_world = get_str("default.world", "");
	second_world = get_str("default.second-world", "");
	main_script = get_str("default.main-script", "");
	default_font = get_str("default.font", "");
	default_material = get_str("default.material", "");
	debug_level = get_int("debug.level", 1);

	string aa = get_str("renderer.antialiasing", "");
	if (aa == "") {
	} else if (aa == "MSAA") {
		antialiasing_method = AntialiasingMethod::MSAA;
	} else if (aa == "TAA") {
		antialiasing_method = AntialiasingMethod::TAA;
	} else {
		msg_error("unknown antialiasing method: " + aa);
	}

	resolution_scale_min = get_float("renderer.resolution-scale-min", 0.5f);
	resolution_scale_max = get_float("renderer.resolution-scale-max", 1.0f);
	resolution_scale_filter = get_str("renderer.resolution-scale-filter", "linear");
	target_framerate = get_float("renderer.target-framerate", 60.0f);
	min_framerate = get_float("renderer.min-framerate", 10.0f);

	ambient_occlusion_radius = get_float("renderer.ssao.radius", 10);
	if (!get_bool("renderer.ssao.enabled", true))
		ambient_occlusion_radius = -1;

	additional_scripts.append(get_str_array("default.additional-scripts"));
	if (debug_level >= 1)
		additional_scripts.append(get_str_array("debug.scripts1"));
	if (debug_level >= 2)
		additional_scripts.append(get_str_array("debug.scripts2"));
}

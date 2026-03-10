/*
 * Config.cpp
 *
 *  Created on: 07.08.2020
 *      Author: michi
 */

#include "Config.h"
#include <lib/os/msg.h>
#include <lib/os/CommandLineParser.h>

const string RawConfig::ID_API_VERSION = "api.version";
const string RawConfig::ID_DEFAULT_SCRIPT = "default.main-script";
const string RawConfig::ID_DEFAULT_WORLD = "default.world";
const string RawConfig::ID_DEFAULT_WORLD2 = "default.second-world";
const string RawConfig::ID_DEFAULT_MATERIAL = "default.material";
const string RawConfig::ID_DEFAULT_FONT = "default.font";
const string RawConfig::ID_SCREEN_MODE = "screen.mode";
const string RawConfig::ID_SCREEN_WIDTH = "screen.width";
const string RawConfig::ID_SCREEN_HEIGHT = "screen.height";
const string RawConfig::ID_RENDERER_PATH = "renderer.path";
const string RawConfig::ID_RENDERER_TARGET_FRAMERATE = "renderer.target-framerate";
const string RawConfig::ID_RENDERER_MIN_FRAMERATE = "renderer.min-framerate";
const string RawConfig::ID_RENDERER_ANTIALIASING = "renderer.antialiasing";
const string RawConfig::ID_RENDERER_LIGHTING = "renderer.lighting";
const string RawConfig::ID_RENDERER_LIGHT_SOURCES = "renderer.light-sources";
const string RawConfig::ID_RESOLUTION_SCALE_MIN = "renderer.resolution-scale-min";
const string RawConfig::ID_RESOLUTION_SCALE_MAX = "renderer.resolution-scale-max";
const string RawConfig::ID_RESOLUTION_SCALE_FILTER = "renderer.resolution-scale-filter";
const string RawConfig::ID_SHADOW_BOXSIZE = "shadow.boxsize";
const string RawConfig::ID_SHADOW_RESOLUTION = "shadow.resolution";
const string RawConfig::ID_SHADOW_QUALITY = "shadow.quality";
const string RawConfig::ID_CUBEMAP_RESOLUTION = "cubemap.resolution";
const string RawConfig::ID_CUBEMAP_UPDATE_RATE = "cubemap.update-rate";
const string RawConfig::ID_DEBUG_LEVEL = "debug.level";
const string RawConfig::ID_DEBUG_SCRIPTS1 = "debug.scripts1";
const string RawConfig::ID_DEBUG_SCRIPTS2 = "debug.scripts2";

DigestedConfig config;

RawConfig::RawConfig() {
}

Array<float> parse_range(const string& a) {
	auto xx = a.explode(":");
	if (xx.num >= 2)
		return {xx[0]._float(), xx[1]._float()};
	return {xx[0]._float(), xx[0]._float()};
}

void RawConfig::load(const Array<string> &arg) {
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
		set_int(ID_DEBUG_LEVEL, 2);
	});
	p.option("-D0/--no-debug", "disable debug mode (level 0)", [this] {
		set_int(ID_DEBUG_LEVEL, 0);
	});
	p.option("--game-dir", "DIR", "set game directory", [this] (const string& a) {
		set_str("cli.game-dir", a);
	});
	p.option("--fw/--forward", "use forward rendering path", [this] {
		set_str(ID_RENDERER_PATH, "forward");
	});
	p.option("--def/--deferred", "use deferred rendering path", [this] {
		set_str(ID_RENDERER_PATH, "deferred");
	});
	p.option("--direct", "use direct rendering path", [this] {
		set_str(ID_RENDERER_PATH, "direct");
	});
	p.option("--rt/--raytracing", "use ray tracing rendering path", [this] {
		set_str(ID_RENDERER_PATH, "raytracing");
	});
	p.option("--nortx", "don't use rtx even if available", [this] {
		set_bool("renderer.allow-rtx", false);
	});
	p.option("--msaa", "use multi sampling anti aliasing", [this] {
		set_str(ID_RENDERER_ANTIALIASING, "MSAA");
	});
	p.option("--scale", "RANGE", "use resolutions scale MIN[:MAX]", [this] (const string& a) {
		auto r = parse_range(a);
		set_float(ID_RESOLUTION_SCALE_MIN, r[0]);
		set_float(ID_RESOLUTION_SCALE_MAX, r[1]);
	});
	p.option("--size", "SIZE", "set resolution WxH (-> windowed mode)", [this] (const string& a) {
		auto xx = a.explode("x");
		set_int(ID_SCREEN_WIDTH, xx[0]._int());
		set_int(ID_SCREEN_HEIGHT, xx[1]._int());
		set_str(ID_SCREEN_MODE, "windowed");
	});
	p.option("--fs/--fullscreen", "fullscreen mode", [this] {
		set_str(ID_SCREEN_MODE, "fullscreen");
	});
	p.option("--wfs/--windowed-fullscreen", "windowed fullscreen mode", [this] {
		set_str(ID_SCREEN_MODE, "windowed-fullscreen");
	});
	p.option("--fps", "RANGE", "limit framerate in MIN[:MAX]", [this] (const string& a) {
		auto r = parse_range(a);
		set_float(ID_RENDERER_MIN_FRAMERATE, r[0]);
		set_float(ID_RENDERER_TARGET_FRAMERATE, r[1]);
	});
	p.option("--script", "SCRIPT", "execute additional script", [this] (const string& a) {
		set_str("cli.additional-scripts", a);
	});
	p.cmd("--help", "", "show help", [&p] (const Array<string>& a) {
		p.show();
		exit(0);
	});
	p.cmd("", "[WORLD]", "run game (optionally select first world)", [this, &p] (const Array<string>& a) {
		if (a.num > 0)
			set_str(ID_DEFAULT_WORLD, a[0]);
	});
	p.parse(arg);
	if (p.error)
		exit(1);

	// deprecated
	if (has("default-world") and !has(ID_DEFAULT_WORLD))
		set_str(ID_DEFAULT_WORLD, get_str("default-world", ""));
	if (has("second-world") and !has(ID_DEFAULT_WORLD2))
		set_str(ID_DEFAULT_WORLD2, get_str("second-world", ""));
	if (has("default-font") and !has(ID_DEFAULT_FONT))
		set_str(ID_DEFAULT_FONT, get_str("default-font", ""));
	if (has("default-material") and !has(ID_DEFAULT_MATERIAL))
		set_str(ID_DEFAULT_MATERIAL, get_str("default-material", ""));
	if (has("main-script") and !has(ID_DEFAULT_SCRIPT))
		set_str(ID_DEFAULT_SCRIPT, get_str("main-script", ""));
}

DigestedConfig RawConfig::digest() const {
	DigestedConfig c;

	c.api_version = get_str(ID_API_VERSION, "0");

	c.default_world = get_str(ID_DEFAULT_WORLD, "");
	c.second_world = get_str(ID_DEFAULT_WORLD2, "");
	c.main_script = get_str(ID_DEFAULT_SCRIPT, "");

#ifdef OS_WINDOWS
	c.default_font = get_str(ID_DEFAULT_FONT, "Arial");
#else
	c.default_font = get_str(ID_DEFAULT_FONT, "NotoSans");
#endif
	c.default_material = get_str(ID_DEFAULT_MATERIAL, "");
	c.debug_level = get_int(ID_DEBUG_LEVEL, 1);

	c.renderer_path = get_str(ID_RENDERER_PATH, "forward");
	c.allow_rtx = get_bool("renderer.allow-rtx", true);

	c.ignore_missing_files = get_str("error.missing-files", "ignore") == "ignore";

	c.screen_width = get_int(ID_SCREEN_WIDTH, 1024);
	c.screen_height = get_int(ID_SCREEN_HEIGHT, 768);
	string sm = get_str(ID_SCREEN_MODE, "windowed");
	c.screen_mode = ScreenMode::Windowed;
	if (sm == "fullscreen")
		c.screen_mode = ScreenMode::Fullscreen;
	else if (sm == "windowed-fullscreen")
		c.screen_mode = ScreenMode::WindowedFullscreen;

	string aa = get_str(ID_RENDERER_ANTIALIASING, "");
	if (aa == "") {
	} else if (aa == "MSAA") {
		c.antialiasing_method = AntialiasingMethod::MSAA;
	} else if (aa == "TAA") {
		c.antialiasing_method = AntialiasingMethod::TAA;
	} else {
		msg_error("unknown antialiasing method: " + aa);
	}

	c.lighting = get_str(ID_RENDERER_LIGHTING, "pbr");
	c.light_sources = get_str(ID_RENDERER_LIGHT_SOURCES, "default");
	c.resolution_scale_min = get_float(ID_RESOLUTION_SCALE_MIN, 0.5f);
	c.resolution_scale_max = get_float(ID_RESOLUTION_SCALE_MAX, 1.0f);
	c.resolution_scale_filter = get_str(ID_RESOLUTION_SCALE_FILTER, "linear");
	c.target_framerate = get_float(ID_RENDERER_TARGET_FRAMERATE, 60.0f);
	c.min_framerate = get_float(ID_RENDERER_MIN_FRAMERATE, 10.0f);
	c.uncapped_framerate = get_bool("renderer.uncapped-framerate", false);

	c.shadow_box_size = get_float(ID_SHADOW_BOXSIZE, 2000);
	c.shadow_resolution = get_int(ID_SHADOW_RESOLUTION, 1024);
	c.shadow_quality = get_str(ID_SHADOW_QUALITY, "pcf-hardening");

	c.cubemap_resolution = get_int(ID_CUBEMAP_RESOLUTION, 256);
	c.cubemap_update_rate = get_int(ID_CUBEMAP_UPDATE_RATE, 9);

	c.ambient_occlusion_radius = get_float("renderer.ssao.radius", 10);
	if (!get_bool("renderer.ssao.enabled", true))
		c.ambient_occlusion_radius = -1;

	if (has("cli.game-dir"))
		c.game_dir = get_str("cli.game-dir");

	if (has("cli.additional-scripts"))
		c.additional_scripts.add(get_str("cli.additional-scripts"));

	c.additional_scripts.append(get_str_array("default.additional-scripts"));
	if (c.debug_level >= 1)
		c.additional_scripts.append(get_str_array(ID_DEBUG_SCRIPTS1));
	if (c.debug_level >= 2)
		c.additional_scripts.append(get_str_array(ID_DEBUG_SCRIPTS2));
	return c;
}

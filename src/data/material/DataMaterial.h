/*
 * DataMaterial.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef DATAMATERIAL_H_
#define DATAMATERIAL_H_

#include "../Data.h"
#include <y/world/Material.h>
#include <graphics-impl.h>

namespace nix {
	class Texture;
	class CubeMap;
	class Shader;
	enum class Alpha;
};

class ShaderGraph;

class DataMaterial: public Data {
public:
	DataMaterial(Session *s, bool with_graph = true);
	virtual ~DataMaterial();

	void reset() override;

	void apply_for_rendering() const;

	struct ShaderData {
		Path file;
		string code;
		ShaderGraph *graph = nullptr;
		bool from_graph;
		bool is_default;
		void load_from_file(Session *s);
		void set_engine_default(Session *s);
		void save_to_file(Session *s);

		void reset(Session *s);
	};

	struct RenderPassData {
		ShaderData shader;

		TransparencyMode mode = TransparencyMode::NONE;
		nix::Alpha source = nix::Alpha::ONE, destination = nix::Alpha::ONE;
		float factor = 1.0f;
		bool z_buffer = true;
		int culling = 1;
	};


	struct AppearanceData {
		Array<Path> texture_files;

		// color
		color albedo, emissive;
		float roughness, metal;

		Array<RenderPassData> passes;

		void reset(Session *session);
	};
	AppearanceData appearance;

	struct PhysicsData {
		float friction_jump, friction_static, friction_sliding, friction_rolling;
		float vmin_jump, vmin_sliding;
		// ...
		bool Burnable;
		float BurningTemperature, BurningIntensity;

		void reset();
	};
	PhysicsData physics;

	struct SoundData {
		int NumRules;

		void reset();
	};
	SoundData Sound;
};

#endif /* DATAMATERIAL_H_ */

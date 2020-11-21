/*
 * DataMaterial.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef DATAMATERIAL_H_
#define DATAMATERIAL_H_

#include "../Data.h"
#include "../../x/Material.h"

namespace nix {
	class Texture;
	class CubeMap;
	class Shader;
};

class ShaderGraph;

class DataMaterial: public Data {
public:
	DataMaterial(bool with_graph = true);
	virtual ~DataMaterial();

	void reset() override;

	void apply_for_rendering();


	struct AppearanceData {
		// properties
		Array<Path> texture_files;

		// color
		color albedo, emissive;
		float roughness, reflectivity;

		// transparency
		int transparency_mode, alpha_source, alpha_destination;
		float alpha_factor;
		bool alpha_z_buffer;

		// reflection
		int reflection_mode, reflection_size;
		float reflection_density;
		Array<Path> reflection_texture_file;

		void reset();
	};
	AppearanceData appearance;



	struct ShaderData {
		Path file;
		string code;
		ShaderGraph *graph;
		bool from_graph;
		bool is_default;
		void load_from_file();
		void set_engine_default();
		void save_to_file();

		void reset();
	};
	ShaderData shader;

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

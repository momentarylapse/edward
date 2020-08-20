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
	DataMaterial();
	virtual ~DataMaterial();

	void reset() override;

	void apply_for_rendering();


	struct AppearanceData {
		// properties
		Array<Path> texture_files;

		// color
		color diffuse, emissive;
		float ambient, specular, shininess;

		// transparency
		int transparency_mode, alpha_source, alpha_destination;
		float alpha_factor;
		bool alpha_z_buffer;

		// reflection
		int reflection_mode, reflection_size;
		float reflection_density;
		Path reflection_texture_file[6];

		// shader
		Path shader_file;
		string shader_code;
		ShaderGraph *shader_graph;
		bool shader_from_graph;
		bool is_default_shader;
		void update_shader_from_file();

		void reset();
	};

	AppearanceData appearance;

	struct PhysicsData {
		float friction_jump, friction_static, friction_sliding, friction_rolling;
		float vmin_jump, vmin_sliding;
		// ...
		bool Burnable;
		float BurningTemperature, BurningIntensity;

		void Reset();
	};
	PhysicsData physics;

	struct SoundData {
		int NumRules;

		void Reset();
	};
	SoundData Sound;
};

#endif /* DATAMATERIAL_H_ */

/*
 * DataMaterial.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef DATAMATERIAL_H_
#define DATAMATERIAL_H_

#include "../Data.h"
#include "../../x/material.h"

namespace nix{
	class Texture;
	class CubeMap;
	class Shader;
};

class DataMaterial: public Data
{
public:
	DataMaterial();
	virtual ~DataMaterial();

	void reset() override;

	void ApplyForRendering();
	void UpdateTextures();


	struct AppearanceData {
		// properties
		Array<nix::Texture*> textures;
		Array<string> texture_files;

		// color
		color ambient, diffuse, specular, emissive;
		float shininess;

		// transparency
		int transparency_mode, alpha_source, alpha_destination;
		float alpha_factor;
		bool alpha_z_buffer;

		// reflection
		int reflection_mode, reflection_size;
		float reflection_density;
		string reflection_texture_file[6];
		nix::CubeMap *cube_map;

		// shader
		string shader_file;
		nix::Shader *shader;

		void reset();
		nix::Shader *get_shader() const;
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

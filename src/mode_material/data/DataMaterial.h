/*
 * DataMaterial.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef DATAMATERIAL_H_
#define DATAMATERIAL_H_

#include "../../data/Data.h"
//#include "ShaderGraph.h"
#include <y/world/Material.h>
#include <graphics-impl.h>

class ShaderGraph;

class DataMaterial: public Data {
public:
	explicit DataMaterial(Session *s);
	~DataMaterial() override;

	void reset() override;

	void apply_for_rendering(int pass_no) const;

	struct ShaderData {
		Path file;
		string code;
		ShaderGraph *graph = nullptr;
		bool from_graph = false;
		bool is_default = false;
		void load_from_file(Session *s);
		void set_engine_default(Session *s);
		void save_to_file(Session *s);

		void reset(Session *s);
	};

	struct RenderPassData {
		ShaderData shader;

		TransparencyMode mode = TransparencyMode::NONE;
		Alpha source = Alpha::ONE, destination = Alpha::ONE;
		float factor = 1.0f;
		bool z_buffer = true;
		CullMode culling = CullMode::BACK;
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

		void reset();
	};
	PhysicsData physics;
};

#endif /* DATAMATERIAL_H_ */

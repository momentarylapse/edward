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
#include <lib/yrenderer/Material.h>
#include <lib/ygraphics/graphics-impl.h>

class ShaderGraph;

class DataMaterial: public Data {
public:
	explicit DataMaterial(DocumentSession *doc);
	~DataMaterial() override;

	void reset() override;

	void apply_for_rendering(int pass_no) const;

	struct ShaderData {
		Path file;
		string code;
		ShaderGraph *graph = nullptr;
		bool from_graph = false;
		bool is_default = false;
		void load_from_file(DocumentSession *s);
		void set_engine_default(DocumentSession *s);
		void save_to_file(DocumentSession *s);

		void reset(DocumentSession *s);
	};

	struct RenderPassData {
		ShaderData shader;

		yrenderer::TransparencyMode mode = yrenderer::TransparencyMode::NONE;
		ygfx::Alpha source = ygfx::Alpha::ONE, destination = ygfx::Alpha::ONE;
		float factor = 1.0f;
		bool z_write = true;
		bool z_test = true;
		ygfx::CullMode culling = ygfx::CullMode::BACK;
	};


	struct AppearanceData {
		Array<Path> texture_files;

		// color
		color albedo, emissive;
		float roughness, metal;

		Array<RenderPassData> passes;

		bool cast_shadow = true;

		void reset(DocumentSession* doc);
	};
	AppearanceData appearance;

	struct PhysicsData {
		float friction_jump, friction_static, friction_sliding, friction_rolling;
		float vmin_jump, vmin_sliding;

		void reset();
	};
	PhysicsData physics;

	yrenderer::Material* to_material() const;
	static DataMaterial from_material(DocumentSession* doc, yrenderer::Material* material);
};

#endif /* DATAMATERIAL_H_ */

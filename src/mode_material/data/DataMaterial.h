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

	yrenderer::Material material;

	const yrenderer::Material* to_material() const;
	static DataMaterial from_material(DocumentSession* doc, yrenderer::Material* material);
};

#endif /* DATAMATERIAL_H_ */

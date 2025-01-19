/*
 * ModeModelMeshDeformFunction.h
 *
 *  Created on: 15.12.2014
 *      Author: michi
 */

#ifndef SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHDEFORMFUNCTION_H_
#define SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHDEFORMFUNCTION_H_

#include "../../../ModeCreation.h"
#include "../../../../data/model/DataModel.h"
#include "../../../../lib/base/pointer.h"
#include <y/graphics-fwd.h>

class Geometry;
class ModeModelMesh;
namespace kaba {
	class Context;
	class Module;
}

class ModeModelMeshDeformFunction: public ModeCreation<ModeModelMesh, DataModel> {
public:
	explicit ModeModelMeshDeformFunction(ModeModelMesh *parent);
	~ModeModelMeshDeformFunction() override;

	void on_start() override;
	void on_end() override;

	void on_draw_win(MultiView::Window *win) override;

	void on_preview();
	void on_ok();

private:
	vec3 min, max;
	Geometry *geo;
	Texture *tex;
	owned<kaba::Context> context;
	shared<kaba::Module> script;
	typedef _cdecl vec3 vec_func(const vec3 &);
	vec_func *f;

	enum class CoordSystem {
		RELATIVE,
		RELATIVE_SYM,
		ABSOLUTE
	} coord_system;

	void update_function();
	vec3 transform(const vec3 &v);
	void apply();
	void preview();
	void restore();

	bool has_preview;
	Array<int> index;
	Array<vec3> old_pos;
};

#endif /* SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHDEFORMFUNCTION_H_ */

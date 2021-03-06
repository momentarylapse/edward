/*
 * ModeModelMeshDeformFunction.h
 *
 *  Created on: 15.12.2014
 *      Author: michi
 */

#ifndef SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHDEFORMFUNCTION_H_
#define SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHDEFORMFUNCTION_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../lib/base/pointer.h"

class Geometry;
namespace kaba {
	class Script;
}
namespace nix {
	class NixTexture;
};

class ModeModelMeshDeformFunction: public ModeCreation<DataModel> {
public:
	ModeModelMeshDeformFunction(ModeBase *parent);
	virtual ~ModeModelMeshDeformFunction();

	void on_start() override;
	void on_end() override;

	void on_draw_win(MultiView::Window *win) override;

	void on_preview();
	void on_ok();

private:
	vector min, max;
	Geometry *geo;
	nix::Texture *tex;
	shared<kaba::Script> script;
	typedef _cdecl vector vec_func(const vector &);
	vec_func *f;

	enum class CoordSystem {
		RELATIVE,
		RELATIVE_SYM,
		ABSOLUTE
	} coord_system;

	void update_function();
	vector transform(const vector &v);
	void apply();
	void preview();
	void restore();

	bool has_preview;
	Array<int> index;
	Array<vector> old_pos;
};

#endif /* SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHDEFORMFUNCTION_H_ */

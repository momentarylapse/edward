//
// Created by michi on 10/30/25.
//

#pragma once

#include <action/Action.h>
#include <lib/base/set.h>

struct ModelMesh;
struct vec3;
enum class MultiViewType;

class ActionModelAlignToGrid: public Action {
public:
	explicit ActionModelAlignToGrid(ModelMesh* m, const Data::Selection& selection, const std::function<vec3(const vec3&)>& f);
	string name() override { return "ModelAlignToGrid"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;
private:
	ModelMesh* mesh;
	Array<int> index;
	Array<vec3> old_pos;
};


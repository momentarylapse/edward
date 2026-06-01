//
// Created by michi on 10/30/25.
//

#pragma once

#include <lib/history/Action.h>
#include <lib/base/set.h>
#include <stuff/Selection.h>

struct ModelMesh;
struct vec3;
enum class MultiViewType;

class ActionModelAlignToGrid: public history::Action {
public:
	explicit ActionModelAlignToGrid(ModelMesh* m, const Selection& selection, const std::function<vec3(const vec3&)>& f);
	string name() const override { return "ModelAlignToGrid"; }

	void *execute(history::Data* d) override;
	void undo(history::Data* d) override;
private:
	ModelMesh* mesh;
	Array<int> index;
	Array<vec3> old_pos;
};


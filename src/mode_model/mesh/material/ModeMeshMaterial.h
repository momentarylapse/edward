//
// Created by Michael Ankele on 2025-04-16.
//

#ifndef MODEMESHMATERIAL_H
#define MODEMESHMATERIAL_H


#include <view/Mode.h>
#include "../../data/DataModel.h"
#include <lib/mesh/PolygonMesh.h>
#include <lib/base/optional.h>

class MultiViewWindow;
class ModeMesh;

class ModeMeshMaterial : public SubMode {
public:
	explicit ModeMeshMaterial(ModeMesh* parent);

	void on_enter() override;
	void on_leave() override;
	void on_key_down(int key) override;
	void on_command(const string& id) override;

	ModeMesh* mode_mesh;
	Mode* get_parent() override;

	DataModel* data;
};



#endif //MODEMESHMATERIAL_H

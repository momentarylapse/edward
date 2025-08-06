//
// Created by michi on 04.11.23.
//

#include "SceneView.h"
#include "Light.h"
#include <lib/base/iter.h>

namespace yrenderer {

void SceneView::choose_lights(const Array<Light*>& all_lights) {
	lights.clear();
	for (auto l: all_lights) {
		if (l->enabled) {
			l->light.dir = l->_ang * vec3::EZ;
			lights.add(l);
		}
	}
}

void SceneView::choose_shadows() {
	shadow_indices.clear();
	for (auto&& [i,l]: enumerate(lights)) {
		l->light.shadow_index = -1;
		if (l->allow_shadow and shadow_indices.num == 0) {
			l->light.shadow_index = shadow_indices.num;
			shadow_indices.add(i);
		}
	}
}

}

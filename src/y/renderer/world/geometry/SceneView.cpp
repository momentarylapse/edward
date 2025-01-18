//
// Created by michi on 04.11.23.
//

#include "SceneView.h"
#include <graphics-impl.h>
#include <world/World.h>
#include <world/Light.h>
#include <world/Terrain.h>
#include <helper/PerformanceMonitor.h>
#include <y/ComponentManager.h>
#include <lib/os/time.h>
//#include <lib/threads/Thread.h>
//#include <atomic>


void SceneView::choose_lights() {
	lights.clear();
	shadow_index = -1;
	auto& all_lights = ComponentManager::get_list_family<Light>();
	for (auto l: all_lights) {
		if (!l->enabled)
			continue;

		if (l->allow_shadow) {
			shadow_index = lights.num;
		}
		lights.add(l);
	}
}

/*class TerrainUpdateThread: public Thread {
public:
	void on_run() override {

	}

	std::atomic<Terrain*> terrain = nullptr;
};*/

void SceneView::check_terrains(const vec3& cam_pos) {
	auto& terrains = ComponentManager::get_list_family<Terrain>();
	if (terrains.num == 0)
		return;

	/*if (!terrain_update_thread) {
		terrain_update_thread = new TerrainUpdateThread();
		terrain_update_thread->run();
	}*/

	if (updater.num == 0) {
		auto u = new XTerrainVBUpdater;
		u->terrain = terrains[0];
		u->vb = new VertexBuffer("3f,3f,2f");;
		updater.add(u);

		// first time: complete update!
		terrains[0]->prepare_draw(cam_pos);
		return;
	}

	os::Timer timer;
	for (auto u: updater) {
		while (timer.peek() < 0.0003f) {
			int r = u->iterate(cam_pos);
			if (r == 0)
				break;
			if (r == 2) {
				auto vb = u->vb;
				u->vb = terrains[0]->vertex_buffer.give();
				terrains[0]->vertex_buffer = vb;
				break;
			}
		}
	}

	/*for (auto *t: terrains) {
		t->prepare_draw(cam_pos);
	}*/

}

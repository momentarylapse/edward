#pragma once

#include <lib/base/base.h>
#include <lib/base/map.h>
#include <lib/base/set.h>
#include <lib/base/pointer.h>
#include <lib/ygraphics/graphics-fwd.h>
#include <lib/pattern/Observable.h>


class Session;
namespace yrenderer {
	class Material;
}

class MaterialPreviewManager : public obs::Node<VirtualBase> {
public:
	explicit MaterialPreviewManager(Session*);
	~MaterialPreviewManager() override;


	string get(yrenderer::Material* material);
	void update();

	Session* session;
	base::map<yrenderer::Material*, shared<ygfx::Texture>> textures;
	shared<ygfx::DepthBuffer> depth_buffer;
	base::set<yrenderer::Material*> dirty;

	ygfx::Texture* get_mat_texture(yrenderer::Material* m);

	static constexpr int PREVIEW_SIZE = 48;
};


#pragma once

#include "xhui.h"
#include <lib/image/ImagePainter.h>
#include <lib/math/vec2.h>
#include <lib/math/mat4.h>
#include <lib/ygraphics/font.h>
#include <lib/ygraphics/graphics-fwd.h>
#include <lib/ygraphics/Painter.h>


#if HAS_LIB_VULKAN
namespace vulkan {
	class CommandBuffer;
	class DescriptorSet;
}
#endif

namespace ygfx {
	struct Face;
}


namespace xhui {

class Window;
class Context;
struct XImage;

class Painter : public ygfx::Painter {
public:
	explicit Painter(Context* c, Window *w, const rect& native_area, const rect& area);
	//virtual ~Painter();

	void set_font(const string &font, float size, bool bold, bool italic) override;
	void draw_ximage(const rect& r, const XImage *image);

	void prepare_2d_drawing();

	Context* context = nullptr;
};

}

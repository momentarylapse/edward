/*
 * RendererFactory.h
 *
 *  Created on: 11 Oct 2023
 *      Author: michi
 */

#ifndef SRC_RENDERER_HELPER_RENDERERFACTORY_H_
#define SRC_RENDERER_HELPER_RENDERERFACTORY_H_

struct GLFWwindow;
class Camera;

class RenderPath;

namespace yrenderer {
	class Context;
}

// everything except the camera:
//   window <- region <- ui
void create_base_renderer(yrenderer::Context* ctx, GLFWwindow* window);

// camera (plug into region)
RenderPath* create_render_path(yrenderer::Context* ctx, Camera* cam);
void create_and_attach_render_path(yrenderer::Context* ctx, Camera* cam);

#endif /* SRC_RENDERER_HELPER_RENDERERFACTORY_H_ */

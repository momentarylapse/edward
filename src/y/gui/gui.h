/*
 * gui.h
 *
 *  Created on: Aug 11, 2020
 *      Author: michi
 */

#pragma once


#include <lib/base/pointer.h>
#include <functional>

struct vec2;

namespace ygfx {
	class TextCache;
}

namespace gui {

class Node;

//void init(vulkan::RenderPass *rp);
void init(int ch_iter);
void reset();
//void render(vulkan::CommandBuffer *cb, const rect &viewport);
void update(float aspect_ratio);
void handle_input(const vec2 &m, std::function<void(Node *n)> f);
void handle_mouse_move(const vec2 &m_prev, const vec2 &m);
void iterate(float dt);

extern shared<Node> toplevel;
extern Array<Node*> all_nodes;
void update_tree();

void delete_node(Node *n);

Node* create_node(const string& type);

extern ygfx::TextCache* text_cache;
extern float ui_scale;

}




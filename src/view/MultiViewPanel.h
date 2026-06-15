//
// Created by michi on 6/15/26.
//

#ifndef EDWARD_MULTIVIEWPANEL_H
#define EDWARD_MULTIVIEWPANEL_H


#include <lib/xhui/Panel.h>

namespace yrenderer {
	class XhuiRenderer;
}

class MultiViewRenderer;
class DocumentSession;
class MultiView;

class MultiViewPanel : public xhui::Panel {
public:
	explicit MultiViewPanel(DocumentSession* doc, MultiView* mv);
	~MultiViewPanel() override;

	void set_multi_view(MultiView* mv);

	MultiView* multi_view;
	DocumentSession* doc;
	xhui::Window* win;
	Array<int> events, win_events;

	yrenderer::XhuiRenderer* renderer = nullptr;
	MultiViewRenderer* multi_view_renderer = nullptr;
};

#endif //EDWARD_MULTIVIEWPANEL_H

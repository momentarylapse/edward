//
// Created by michi on 2/24/26.
//

#pragma once

#include <lib/xhui/Panel.h>
#include <lib/pattern/Observable.h>

class Session;
class DocumentSession;
class EdwardWindow;

class ProjectPanel : public obs::Node<xhui::Panel> {
public:
	explicit ProjectPanel(EdwardWindow* window);
	~ProjectPanel() override;

	EdwardWindow* editor_window;
	Session* session;
};

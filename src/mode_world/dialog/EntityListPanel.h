//
// Created by michi on 04/10/2025.
//

#pragma once

#include <lib/xhui/Panel.h>

struct WorldEntity;
class ModeWorld;

class EntityListPanel : public xhui::Panel {
public:
    explicit EntityListPanel();
    void update(ModeWorld* mode);
};

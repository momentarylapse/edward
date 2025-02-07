//
// Created by Michael Ankele on 2025-02-07.
//

#ifndef MODEMODEL_H
#define MODEMODEL_H


#include <lib/base/optional.h>
#include "../view/Mode.h"
#include "../view/Hover.h"
#include "data/DataModel.h"

class Light;
class MultiViewWindow;
class EntityPanel;
namespace xhui {
class Panel;
}

class ModeModel : public Mode {
public:
	explicit ModeModel(Session* session);
	~ModeModel() override;

	void on_enter() override;

	DataModel* data;
};



#endif //MODEMODEL_H

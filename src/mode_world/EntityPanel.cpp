//
// Created by michi on 02.02.25.
//

#include "EntityPanel.h"

EntityPanel::EntityPanel() : Panel("entity-panel") {
	from_source(R"foodelim(
Dialog entity-panel ''
	Grid ? ''
		Button a 'test'
)foodelim");
	expand_x = false;
	min_width_user = 400;
}



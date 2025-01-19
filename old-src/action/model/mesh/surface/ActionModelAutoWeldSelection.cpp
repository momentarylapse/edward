/*
 * ActionModelAutoWeldSelection.cpp
 *
 *  Created on: 25.09.2012
 *      Author: michi
 */

#include "ActionModelAutoWeldSelection.h"
#include "ActionModelSurfaceAutoWeld.h"
#include "../../../../data/model/DataModel.h"
#include "../../../../lib/os/msg.h"

ActionModelAutoWeldSelection::ActionModelAutoWeldSelection(float _epsilon)
{
	epsilon = _epsilon;
}

void *ActionModelAutoWeldSelection::compose(Data *d)
{
	msg_todo("auto weld...");
#if 0
	DataModel *m = dynamic_cast<DataModel*>(d);
	bool found = true;
	while (found){
		found = false;
		for (int i=m->surface.num-1;i>=0;i--)
			if (m->surface[i].is_selected){
				m->surface[i].testSanity("auto weld pre");
				for (int j=i-1;j>=0;j--)
					if (m->surface[j].is_selected){
						if (addSubAction(new ActionModelSurfaceAutoWeld(j, i, epsilon), m)){
							//msg_write("ok");
							found = true;
							break;
						}
					}
				if (found)
					break;
			}
	}
#endif
	return NULL;
}


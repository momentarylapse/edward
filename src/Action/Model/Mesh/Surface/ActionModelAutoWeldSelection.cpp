/*
 * ActionModelAutoWeldSelection.cpp
 *
 *  Created on: 25.09.2012
 *      Author: michi
 */

#include "ActionModelAutoWeldSelection.h"
#include "ActionModelSurfaceAutoWeld.h"
#include "../../../../Data/Model/DataModel.h"

ActionModelAutoWeldSelection::ActionModelAutoWeldSelection(DataModel *m, float d)
{
	bool found = true;
	while (found){
		found = false;
		for (int i=m->Surface.num-1;i>=0;i--){
			for (int j=i-1;j>=0;j--)
				if (m->Surface[i].is_selected)
					if (m->Surface[j].is_selected){
						Action *a = new ActionModelSurfaceAutoWeld(m, j, i, d, false);
						try{
							AddSubAction(a, m);
							//msg_write("ok");
							found = true;
							break;
						}catch(ActionException &e){
							//msg_write("abort... " + e.message);
						}
					}
			if (found)
				break;
		}
	}
}

ActionModelAutoWeldSelection::~ActionModelAutoWeldSelection()
{
}


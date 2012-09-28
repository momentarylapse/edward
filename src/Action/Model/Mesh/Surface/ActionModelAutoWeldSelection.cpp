/*
 * ActionModelAutoWeldSelection.cpp
 *
 *  Created on: 25.09.2012
 *      Author: michi
 */

#include "ActionModelAutoWeldSelection.h"
#include "ActionModelSurfaceAutoWeld.h"
#include "../../../../Data/Model/DataModel.h"

ActionModelAutoWeldSelection::ActionModelAutoWeldSelection(float _epsilon)
{
	epsilon = _epsilon;
}

void *ActionModelAutoWeldSelection::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	bool found = true;
	while (found){
		found = false;
		for (int i=m->Surface.num-1;i>=0;i--)
			if (m->Surface[i].is_selected){
				m->Surface[i].TestSanity("auto weld pre");
				for (int j=i-1;j>=0;j--)
					if (m->Surface[j].is_selected){
						try{
							AddSubAction(new ActionModelSurfaceAutoWeld(j, i, epsilon, false), m);
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
	return NULL;
}


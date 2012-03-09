/*
 * MaterialPropertiesDialog.h
 *
 *  Created on: 09.03.2012
 *      Author: michi
 */

#ifndef MATERIALPROPERTIESDIALOG_H_
#define MATERIALPROPERTIESDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../Data/Material/DataMaterial.h"

class MaterialPropertiesDialog: public CHuiWindow
{
public:
	MaterialPropertiesDialog(CHuiWindow *_parent, bool _allow_parent, DataMaterial *_data);
	virtual ~MaterialPropertiesDialog();

private:
	DataMaterial *data;
};

#endif /* MATERIALPROPERTIESDIALOG_H_ */

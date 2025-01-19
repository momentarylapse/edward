/*
 * MaterialRenderPassDialog.h
 *
 *  Created on: 9 Nov 2023
 *      Author: michi
 */

#ifndef SRC_MODE_MATERIAL_DIALOG_MATERIALRENDERPASSDIALOG_H_
#define SRC_MODE_MATERIAL_DIALOG_MATERIALRENDERPASSDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../data/material/DataMaterial.h"

class MaterialRenderPassDialog: public hui::Dialog {
public:
	MaterialRenderPassDialog(hui::Window *_parent, const DataMaterial::RenderPassData &data);

	void load_data();
	void on_transparency_mode();

	void on_ok();

	DataMaterial::RenderPassData result;
	bool success = false;
};

#endif /* SRC_MODE_MATERIAL_DIALOG_MATERIALRENDERPASSDIALOG_H_ */

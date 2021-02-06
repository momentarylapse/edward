/*
 * FormatModel.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "FormatModel.h"
#include "../../Data/Model/DataModel.h"
#include "../../Data/Model/ModelMesh.h"
#include "../../Data/Model/ModelPolygon.h"
#include "../../Mode/Model/ModeModel.h"
#include "../../Edward.h"
#include "../../x/ModelManager.h"
#include "../../lib/xfile/chunked.h"

FormatModel::FormatModel() : TypedFormat<DataModel>(FD_MODEL, "model", _("Model"), Flag::CANONICAL_READ_WRITE) {
}

const bool write_external_edit_file = false;


void update_model_script_data(DataModel::MetaData &m);

bool DataModelAllowUpdating = true;


int get_normal_index(vector &n) {
	int nxy, nz;
	if ((n.x == 0) and (n.y == 0)) {
		nxy = 0;
		nz = (n.z < 0) ? 255 : 0;
	} else {
		nxy = (int)(atan2(n.y, n.x) / pi / 2 * 255.0f) & 255;
		nz = (int)(acos(n.z) / pi * 255.0f) & 255;
	}
	return nxy + 256 * nz;
}

vector get_normal_by_index(int index) {
	float wz = (float)(index >> 8) * pi / 255.0f;
	float wxy = (float)(index & 255) * 2 * pi / 255.0f;
	float swz = sin(wz);
	if (swz < 0)
		swz = - swz;
	float cwz = cos(wz);
	return vector( cos(wxy) * swz, sin(wxy) * swz, cwz);
}



char read_first_char(const Path &filename) {
	File *f = FileOpen(filename);
	return f->read_char();
}

class ModelParser : public ChunkedFileParser {
public:
	ModelParser() : ChunkedFileParser(8) {

	}
};

void FormatModel::_load(const Path &filename, DataModel *data, bool deep) {

	char c = read_first_char(filename);
	data->file_time = file_mtime(filename).time;

	if (c == 'b' or c == 't') {
		_load_old(filename, data, deep);
	} else {
		ModelParser p;
		p.read(filename, data);
	}








	if (deep){

		// import...
		if (data->mesh->polygon.num == 0)
			data->importFromTriangleSkin(1);

		for (ModelMove &m: data->move)
			if (m.type == MOVE_TYPE_VERTEX){
				for (ModelFrame &f: m.frame)
					f.vertex_dpos = f.skin[1].dpos;
			}

		for (auto *m: data->material){
			m->make_consistent();

			// test textures
			for (auto &t: m->texture_levels){
				if ((!t->texture) and (!t->filename.is_empty()))
					warning(format(_("Texture file not loadable: %s"), t->filename));
			}
		}



		// TODO -> mode...
		/*if (this == mode_model->data){
			ed->SetTitle(filename);
			ResetView();
		}*/
	}

	// FIXME
	if ((!data->meta_data.script_file.is_empty()) and (data->meta_data.variables.num == 0)){
		update_model_script_data(data->meta_data);
		msg_write(data->meta_data.variables.num);
		for (int i=0; i<min(data->meta_data.script_var.num, data->meta_data.variables.num); i++){
			if (data->meta_data.variables[i].type == "float")
				data->meta_data.variables[i].value = f2s(data->meta_data.script_var[i], 6);
			msg_write(format("  try import var  %s = %s", data->meta_data.variables[i].name, data->meta_data.variables[i].value));
		}
	}



	if (deep)
		data->on_post_action_update();
}

Set<int> get_all_poly_vert(DataModel *m) {
	Set<int> all_vert;
	for (auto &p: m->phys_mesh->polygon)
		for (auto &f: p.side)
			all_vert.add(f.vertex);
	return all_vert;
}

Array<Set<int>> split_conv_polyhedra(DataModel *m) {
	auto all_vert = get_all_poly_vert(m);

	Array<Set<int>> surf;

	while(all_vert.num > 0) {
		msg_write("----surf");
		Set<int> cur;
		cur.add(all_vert.pop());
		int n = 0;
		while (cur.num > n) {
			n = cur.num;

			for (auto &e: m->phys_mesh->edge) {
				if (cur.contains(e.vertex[0]) and all_vert.contains(e.vertex[1])) {
					cur.add(e.vertex[1]);
					all_vert.erase(e.vertex[1]);
				}
				if (cur.contains(e.vertex[1]) and all_vert.contains(e.vertex[0])) {
					cur.add(e.vertex[0]);
					all_vert.erase(e.vertex[0]);
				}
			}
		}
		msg_write(ia2s(cur));
		surf.add(cur);
	}
	return surf;
}

Array<ModelPolygon> conv_poly_poly(DataModel *m, const Set<int> &v) {
	Array<ModelPolygon> poly;
	for (auto &p: m->phys_mesh->polygon)
		if (v.contains(p.side[0].vertex))
			poly.add(p);
	return poly;
}

void FormatModel::_save(const Path &filename, DataModel *data) {
	if (DataModelAllowUpdating){
		/*if (AutoGenerateSkin[1])
			CreateSkin(&Skin[1],&Skin[2],(float)DetailFactor[1]*0.01f);

		if (AutoGenerateSkin[2])
			CreateSkin(&Skin[2],&Skin[3],(float)DetailFactor[2]/(float)DetailFactor[1]);*/

		if (data->meta_data.auto_generate_dists)
			data->generateDetailDists(data->meta_data.detail_dist);

		if (data->meta_data.auto_generate_tensor)
			data->meta_data.inertia_tensor = data->generateInertiaTensor(data->meta_data.mass);



	#ifdef FORCE_UPDATE_NORMALS
		for (int d=1;d<4;d++)
			for (int j=0;j<skin[d].NumVertices;j++)
				skin[d].vertex[j].normal_dirty = true;
	#endif
		data->updateNormals();

		// export...
		data->mesh->export_to_triangle_skin(data->skin[1]);
		for (int d=1;d<4;d++){
			if (data->skin[d].sub.num != data->material.num){
				data->skin[d].sub.resize(data->material.num);
			}
		}


	//	PrecreatePhysicalData();
	}


	// so the materials don't get mixed up
//	RemoveUnusedData();

	//_save_v11_poly(filename.with(".edit"), data);
	_save_v11(filename, data);

	if (write_external_edit_file)
		_save_v11_edit(filename, data);

}

void _save_v11_edit_x(File *f, DataModel *data) {

	// additional data for editing
	f->write_str("// Editor");
	f->write_bool(data->meta_data.auto_generate_tensor);
	f->write_bool(data->meta_data.auto_generate_dists);
	f->write_bool(data->meta_data.auto_generate_skin[1]);
	f->write_bool(data->meta_data.auto_generate_skin[2]);
	f->write_int(data->meta_data.detail_factor[1]);
	f->write_int(data->meta_data.detail_factor[2]);
	f->write_str("// Normals");
	for (int i=1;i<4;i++){
		ModelSkin *s = &data->skin[i];
		f->write_int(NORMAL_MODE_PER_VERTEX);
		for (ModelVertex &v: s->vertex)
			f->write_int(v.normal_mode);
	}
	f->write_str("// Polygons");
	f->write_int(1);
	f->write_int(data->mesh->polygon.num);
	for (auto &t: data->mesh->polygon){
		f->write_int(t.side.num);
		f->write_int(t.material);
		for (ModelPolygonSide &ss: t.side){
			f->write_int(ss.vertex);
			for (int l=0;l<data->material[t.material]->texture_levels.num;l++){
				f->write_float(ss.skin_vertex[l].x);
				f->write_float(ss.skin_vertex[l].y);
			}
		}
	}
	f->write_bool(false);
	f->write_bool(true);
	f->write_int(0);

	f->write_str("#");
}

void FormatModel::_save_v11(const Path &filename, DataModel *data) {

	File *f = FileCreate(filename);
	f->WriteFileFormatVersion(true, 11);//FFVBinary, 11);
	f->float_decimals = 5;

// general
	f->write_comment("// General");
	vector _min, _max;
	data->getBoundingBox(_min, _max);
	f->write_vector(&_min);
	f->write_vector(&_max);
	f->write_int(3); // skins...
	f->write_int(0); // reserved
	f->write_int(0);
	f->write_int(0);

// materials
	f->write_comment("// Materials");
	f->write_int(data->material.num);
	for (ModelMaterial *m: data->material){
		f->write_str(m->filename.str());
		f->write_bool(m->col.user);
		write_color_argb(f, m->col.ambient());
		write_color_argb(f, m->col.albedo);
		write_color_argb(f, m->col.specular());
		write_color_argb(f, m->col.emission);
		f->write_int(m->col.shininess());
		f->write_int(m->alpha.user ? m->alpha.mode : TransparencyModeDefault);
		f->write_int(m->alpha.source);
		f->write_int(m->alpha.destination);
		f->write_int(m->alpha.factor * 100.0f);
		f->write_bool(m->alpha.zbuffer);
		f->write_int(m->texture_levels.num);
		for (int t=0;t<m->texture_levels.num;t++)
			f->write_str(m->texture_levels[t]->filename.str());
	}

// physical skin
	f->write_comment("// Physical Skin");

	// vertices
	f->write_int(data->phys_mesh->vertex.num);
	for (int j=0;j<data->phys_mesh->vertex.num;j++)
		f->write_int(data->phys_mesh->vertex[j].bone_index);
	for (int j=0;j<data->phys_mesh->vertex.num;j++)
		f->write_vector(&data->phys_mesh->vertex[j].pos);

	// triangles
	f->write_int(0);
	/*for (int j=0;j<phys_mesh->NumTriangles;j++)
		for (int k=0;k<3;k++)
			f->write_int(phys_mesh->Triangle[j].Index[k]);*/

	// balls
	f->write_int(data->phys_mesh->ball.num);
	for (auto &b: data->phys_mesh->ball){
		f->write_int(b.index);
		f->write_float(b.radius);
	}

	auto surf = split_conv_polyhedra(data);
	f->write_int(surf.num);
	for (auto &pp: surf) {
		auto p = conv_poly_poly(data, pp);

		f->write_int(p.num);
		for (int k=0;k<p.num;k++){
			f->write_int(p[k].side.num);
			for (int l=0;l<p[k].side.num;l++)
				f->write_int(p[k].side[l].vertex);
			f->write_float(0); // plane a,b,c,d
			f->write_float(0);
			f->write_float(0);
			f->write_float(0);
		}
		/*f->write_int(p.NumSVertices);
		for (int k=0;k<p.NumSVertices;k++)
			f->write_int(p.SIndex[k]);
		f->write_int(p.NumEdges);
		for (int k=0;k<p.NumEdges;k++){
			f->write_int(p.EdgeIndex[k*2 + 0]);
			f->write_int(p.EdgeIndex[k*2 + 1]);
		}*/
		f->write_int(0);
		f->write_int(0);

		// topology
		/*for (int k=0;k<p.NumFaces;k++)
			for (int l=0;l<p.NumFaces;l++)
				f->write_int(p.FacesJoiningEdge[k * p.NumFaces + l]);
		for (int k=0;k<p.NumEdges;k++)
			for (int l=0;l<p.NumFaces;l++)
				f->write_bool(p.EdgeOnFace[k * p.NumFaces + l]);*/
		for (int k=0;k<p.num;k++)
			for (int l=0;l<p.num;l++)
					f->write_int(0);
	}

// skin
	for (int i=1;i<4;i++){
		ModelSkin *s = &data->skin[i];
		f->write_comment(format("// Skin[%d]",i));

		// verices
		f->write_int(s->vertex.num);
		for (ModelVertex &v: s->vertex)
			f->write_vector(&v.pos);
		for (ModelVertex &v: s->vertex)
			f->write_int(v.bone_index);

		// skin vertices
		int num_skin_v = 0;
		for (int m=0;m<data->material.num;m++)
			num_skin_v += s->sub[m].triangle.num * data->material[m]->texture_levels.num * 3;
		f->write_int(num_skin_v);
		for (int m=0;m<data->material.num;m++)
			for (int tl=0;tl<data->material[m]->texture_levels.num;tl++)
				for (int j=0;j<s->sub[m].triangle.num;j++)
					for (int k=0;k<3;k++){
						f->write_float(s->sub[m].triangle[j].skin_vertex[tl][k].x);
						f->write_float(s->sub[m].triangle[j].skin_vertex[tl][k].y);
					}


		// sub skins
		int svi = 0;
		for (int m=0;m<data->material.num;m++){
			ModelSubSkin *sub = &s->sub[m];

			// triangles
			f->write_int(sub->triangle.num);

			// vertex index
			for (int j=0;j<sub->triangle.num;j++)
				for (int k=0;k<3;k++)
					f->write_int(sub->triangle[j].vertex[k]);

			// skin index
			for (int tl=0;tl<data->material[m]->texture_levels.num;tl++)
				for (int j=0;j<sub->triangle.num;j++)
					for (int k=0;k<3;k++)
						f->write_int(svi ++);

			// normal
			for (int j=0;j<sub->triangle.num;j++)
				for (int k=0;k<3;k++){
					if (DataModelAllowUpdating)
						sub->triangle[j].normal_index[k] = get_normal_index(sub->triangle[j].normal[k]);
					f->write_word(sub->triangle[j].normal_index[k]);
				}
			f->write_int(0);
		}

		f->write_int(0);
	}

// skeleton
	f->write_comment("// Skeleton");
	f->write_int(data->bone.num);
	for (ModelBone &b: data->bone){
		if (b.parent >= 0){
			vector dpos = b.pos - data->bone[b.parent].pos;
			f->write_vector(&dpos);
		}else
			f->write_vector(&b.pos);
		f->write_int(b.parent);
		f->write_str(b.model_file.str());
	}

// animations
	f->write_comment("// Animations");
	if ((data->move.num == 1) and (data->move[0].frame.num == 0)){
		f->write_int(0);
	}else
		f->write_int(data->move.num);
	int n_moves = 0;
	int n_frames_vert = 0;
	int n_frames_skel = 0;
	for (int i=0;i<data->move.num;i++)
		if (data->move[i].frame.num > 0){
			n_moves ++;
			if (data->move[i].type == MOVE_TYPE_VERTEX)	n_frames_vert += data->move[i].frame.num;
			if (data->move[i].type == MOVE_TYPE_SKELETAL)	n_frames_skel += data->move[i].frame.num;
		}
	f->write_int(n_moves);
	f->write_int(n_frames_vert);
	f->write_int(n_frames_skel);
	for (int i=0;i<data->move.num;i++)
		if (data->move[i].frame.num > 0){
			ModelMove *m = &data->move[i];
			bool rubber_timing = m->needsRubberTiming();
			f->write_int(i);
			f->write_str(m->name);
			f->write_int(m->type + (rubber_timing ? 128 : 0));
			f->write_int(m->frame.num);
			f->write_float(m->frames_per_sec_const);
			f->write_float(m->frames_per_sec_factor);

			// vertex animation
			if (m->type == MOVE_TYPE_VERTEX){
				for (ModelFrame &fr: m->frame){
					if (rubber_timing)
						f->write_float(fr.duration);
					for (int s=0;s<4;s++){
						// compress (only write != 0)
						int num_vertices = 0;
						for (int j=0;j<data->skin[s].vertex.num;j++)
							if (fr.skin[i].dpos[j] != v_0)
								num_vertices ++;
						f->write_int(num_vertices);
						for (int j=0;j<data->skin[s].vertex.num;j++)
							if (fr.skin[i].dpos[j] != v_0){
								f->write_int(j);
								f->write_vector(&fr.skin[i].dpos[j]);
							}
					}
				}
			// skeletal animation
			}else if (m->type == MOVE_TYPE_SKELETAL){
				for (int j=0;j<data->bone.num;j++)
					f->write_bool((data->bone[j].parent < 0));
				f->write_bool(m->interpolated_quadratic);
				f->write_bool(m->interpolated_loop);
				for (ModelFrame &fr: m->frame){
					if (rubber_timing)
						f->write_float(fr.duration);
					for (int j=0;j<data->bone.num;j++){
						f->write_vector(&fr.skel_ang[j]);
						if (data->bone[j].parent < 0)
							f->write_vector(&fr.skel_dpos[j]);
					}
				}
			}
		}

// effects
	f->write_comment("// Effects");
	f->write_int(data->fx.num);
	for (auto &e: data->fx){
		if (e.type == FX_TYPE_SCRIPT){
			f->write_str("Script");
			f->write_int(e.vertex);
			f->write_str(e.file.str());
			f->write_str("");
		}else if (e.type == FX_TYPE_LIGHT){
			f->write_str("Light");
			f->write_int(e.vertex);
			f->write_int((int)e.size);
			for (int nc=0;nc<3;nc++)
				write_color_argb(f, e.colors[nc]);
		}else if (e.type == FX_TYPE_SOUND){
			f->write_str("Sound");
			f->write_int(e.vertex);
			f->write_int((int)e.size);
			f->write_int((int)(e.speed * 100.0f));
			f->write_str(e.file.str());
		}else if (e.type == FX_TYPE_FORCEFIELD){
			f->write_str("ForceField");
			f->write_int(e.vertex);
			f->write_int((int)e.size);
			f->write_int((int)e.intensity);
			f->write_bool(e.inv_quad);
		}
	}

// properties
	f->write_comment("// Physics");
	f->write_float(data->meta_data.mass);
	for (int i=0;i<9;i++)
		f->write_float(data->meta_data.inertia_tensor.e[i]);
	f->write_bool(data->meta_data.active_physics);
	f->write_bool(data->meta_data.passive_physics);
	f->write_float(data->getRadius());

	f->write_comment("// LOD-Distances");
	f->write_float(data->meta_data.detail_dist[0]);
	f->write_float(data->meta_data.detail_dist[1]);
	f->write_float(data->meta_data.detail_dist[2]);

// object data
	f->write_comment("// Object Data");
	f->write_str(data->meta_data.name);
	f->write_str(data->meta_data.description);

	// inventory
	f->write_comment("// Inventory");
	f->write_int(data->meta_data.inventary.num);
	for (int i=0;i<data->meta_data.inventary.num;i++){
		f->write_str(data->meta_data.inventary[i].str());
		f->write_int(1);
	}

	// script
	f->write_comment("// Script");
	f->write_str(data->meta_data.script_file.str());
	f->write_int(data->meta_data.script_var.num);
	for (int i=0;i<data->meta_data.script_var.num;i++)
		f->write_float(data->meta_data.script_var[i]);

	// new script vars
	if (data->meta_data.variables.num > 0){
		f->write_str("// Script Vars");
		f->write_int(data->meta_data.variables.num);
		for (auto &v: data->meta_data.variables){
			f->write_str(v.name);
			f->write_str(v.value);
		}
	}


	if (data->phys_mesh->cylinder.num > 0){
		f->write_str("// Cylinders");
		f->write_int(data->phys_mesh->cylinder.num);
		for (auto &c: data->phys_mesh->cylinder){
			f->write_int(c.index[0]);
			f->write_int(c.index[1]);
			f->write_float(c.radius);
			f->write_bool(c.round);
		}
	}

	if (write_external_edit_file)
		f->write_str("#");
	else
		_save_v11_edit_x(f, data);

	FileClose(f);
}

void FormatModel::_save_v11_edit(const Path &filename, DataModel *data) {
	File *f = FileCreate(filename.with(".edit"));
	f->WriteFileFormatVersion(true, 11);//FFVBinary, 11);
	f->float_decimals = 5;

	_save_v11_edit_x(f, data);

	FileClose(f);
}

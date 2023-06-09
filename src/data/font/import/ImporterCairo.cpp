/*
 * ImporterCairo.cpp
 *
 *  Created on: 22.03.2013
 *      Author: michi
 */

#include "ImporterCairo.h"
#include "../DataFont.h"
#include "../../../lib/image/image.h"
//#include "../../../lib/nix/nix.h"
#include "../../../lib/os/msg.h"
#include "../../../y/ResourceManager.h"
#include <pango/pangocairo.h>

static const int ImportCairoTrySize[][2] =
{
		{64, 64},
		{128, 64},
		{128, 128},
		{256, 128},
		{256, 256},
		{512, 256},
		{512, 512},
		{1024, 512},
		{1024, 1024},
		{-1, -1}
};

ImporterCairo::ImporterCairo()
{
}

ImporterCairo::~ImporterCairo()
{
}

bool ImporterCairo::Import(DataFont *f, const string &font_name)
{
	msg_write("import " + font_name);
	Image im;
	int n = 0;
	while (ImportCairoTrySize[n][0] > 0){
		bool ok = TryImport(f, font_name, ImportCairoTrySize[n][0], ImportCairoTrySize[n][1], im);
		if (ok){
			f->global.TextureFile = Path("Font") | (font_name + ".tga");
			im.save(ResourceManager::texture_dir | f->global.TextureFile);
			f->out_changed();
			return true;
		}
		n ++;
	}
	f->reset();
	return false;
}

bool ImporterCairo::TryImport(DataFont *f, const string &font_name, int w_tex, int h_tex, Image &im)
{
	bool failed = false;
	cairo_surface_t *surface;
	cairo_t *cr;

	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w_tex, h_tex);
	cr = cairo_create(surface);

	cairo_set_source_rgba(cr, 0, 0, 0, 1);
	cairo_rectangle(cr, 0, 0, w_tex, h_tex);
	cairo_fill(cr);

	int x = 0, y = 0;

	cairo_set_source_rgba(cr, 1, 1, 1, 1);

	PangoLayout *layout = pango_cairo_create_layout(cr);
	PangoFontDescription *desc = pango_font_description_from_string(font_name.c_str());
	pango_layout_set_font_description(layout, desc);
	pango_font_description_free(desc);

	pango_layout_set_text(layout, "Test", -1);
	int baseline = pango_layout_get_baseline(layout) / PANGO_SCALE;
	int w_test, h_test;
	pango_layout_get_pixel_size(layout, &w_test, &h_test);

	f->global.GlyphHeight = h_test;
	f->global.GlyphY2 = baseline;
	f->global.GlyphY1 = baseline / 5;

	for (DataFont::Glyph &g: f->glyph){
		pango_layout_set_text(layout, g.Name.c_str(), -1);
		PangoRectangle r, r_log;
		pango_layout_get_pixel_extents(layout, &r, &r_log);
		cairo_move_to(cr, x - r.x, y);
		g.Width = r.width + 1;
		if (r.width == 0)
			g.Width = r_log.width + 1;
		g.X1 = max(0, -r.x);
		g.X2 = g.Width;
		x += g.Width;
		if (x > w_tex){
			x = 0;
			y += f->global.GlyphHeight;
			cairo_move_to(cr, x - r.x, y);
			x += g.Width;
		}
		pango_cairo_show_layout(cr, layout);
	}
	g_object_unref(layout);

	failed = (y > h_tex);

	if (!failed){
		cairo_surface_flush(surface);
		unsigned char *c = cairo_image_surface_get_data(surface);
		im.create(w_tex, h_tex, White);
		for (int y=0;y<h_tex;y++)
			for (int x=0;x<w_tex;x++){
				float a = (float)c[1] / 255.0f;
				im.set_pixel(x, y, color(a, 1, 1, 1));
				c += 4;
			}
		im.alpha_used = true;
	}

	cairo_destroy(cr);
	cairo_surface_destroy(surface);
	return !failed;
}


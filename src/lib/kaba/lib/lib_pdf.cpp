#include "../kaba.h"
#include "../../config.h"
#include "../../any/any.h"
#include "lib.h"
#include "shared.h"



#if __has_include("../../doc/pdf.h")
	#include "../../doc/pdf.h"
	#define pdf_p(p) p
#else
	namespace pdf {
		typedef int Parser;
	}
	#define pdf_p(p) nullptr
#endif
#if __has_include("../../doc/ttf.h")
	#include "../../doc/ttf.h"
	#define ttf_p(p) p
#else
	#define ttf_p(p) nullptr
#endif


namespace kaba {


extern const Class *TypeBasePainterXfer;
extern const Class *TypePath;

void SIAddPackagePdf(Context *c) {
	add_package(c, "pdf");

	const Class *TypeParser = add_type("Parser", sizeof(pdf::Parser));

	add_class(TypeParser);
		class_add_func(Identifier::func::Init, TypeVoid, pdf_p(&pdf::Parser::__init__), Flags::Mutable);
		class_add_func(Identifier::func::Delete, TypeVoid, pdf_p(&pdf::Parser::__delete__), Flags::Mutable);
		class_add_func("set_page_size", TypeVoid, pdf_p(&pdf::Parser::set_page_size), Flags::Mutable);
			func_add_param("width", TypeFloat32);
			func_add_param("height", TypeFloat32);
		class_add_func("add_page", TypeBasePainterXfer, pdf_p(&pdf::Parser::add_page), Flags::Mutable);
		class_add_func("save", TypeVoid, pdf_p(&pdf::Parser::save));
			func_add_param("filename", TypePath);

	add_func("add_font_directory", TypeVoid, ttf_p(&ttf::add_font_directory), Flags::Static);
		func_add_param("dir", TypePath);

}

};

/*
 * Inline.h
 *
 *  Created on: May 9, 2021
 *      Author: michi
 */

#pragma once

namespace kaba {

enum class InlineID {
	NONE = -1,
	FLOAT_TO_INT,
	FLOAT_TO_FLOAT64,
	FLOAT64_TO_FLOAT,
	INT_TO_FLOAT,
	INT_TO_INT64,
	INT64_TO_INT,
	INT_TO_CHAR,
	CHAR_TO_INT,
	POINTER_TO_BOOL,
	COMPLEX_SET,
	VECTOR_SET,
	RECT_SET,
	COLOR_SET,

	POINTER_ASSIGN,
	POINTER_EQUAL,
	POINTER_NOT_EQUAL,

	CHAR_ASSIGN,
	CHAR_EQUAL,
	CHAR_NOT_EQUAL,
	CHAR_GREATER,
	CHAR_GREATER_EQUAL,
	CHAR_SMALLER,
	CHAR_SMALLER_EQUAL,
	CHAR_ADD,
	CHAR_ADD_ASSIGN,
	CHAR_SUBTRACT,
	CHAR_SUBTRACT_ASSIGN,
	CHAR_AND,
	CHAR_OR,
	CHAR_NEGATIVE,

	BOOL_ASSIGN,
	BOOL_EQUAL,
	BOOL_NOT_EQUAL,
	BOOL_AND,
	BOOL_OR,
	BOOL_NEGATE,

	INT_ASSIGN,
	INT_ADD,
	INT_ADD_ASSIGN,
	INT_SUBTRACT,
	INT_SUBTRACT_ASSIGN,
	INT_MULTIPLY,
	INT_MULTIPLY_ASSIGN,
	INT_DIVIDE,
	INT_DIVIDE_ASSIGN,
	INT_MODULO,
	INT_EQUAL,
	INT_NOT_EQUAL,
	INT_GREATER,
	INT_GREATER_EQUAL,
	INT_SMALLER,
	INT_SMALLER_EQUAL,
	INT_AND,
	INT_OR,
	INT_SHIFT_RIGHT,
	INT_SHIFT_LEFT,
	INT_NEGATIVE,
	INT_INCREASE,
	INT_DECREASE,

	INT64_ASSIGN,
	INT64_ADD,
	INT64_ADD_INT,
	INT64_ADD_ASSIGN,
	INT64_SUBTRACT,
	INT64_SUBTRACT_ASSIGN,
	INT64_MULTIPLY,
	INT64_MULTIPLY_ASSIGN,
	INT64_DIVIDE,
	INT64_DIVIDE_ASSIGN,
	INT64_MODULO,
	INT64_EQUAL,
	INT64_NOT_EQUAL,
	INT64_GREATER,
	INT64_GREATER_EQUAL,
	INT64_SMALLER,
	INT64_SMALLER_EQUAL,
	INT64_AND,
	INT64_OR,
	INT64_SHIFT_RIGHT,
	INT64_SHIFT_LEFT,
	INT64_NEGATIVE,
	INT64_INCREASE,
	INT64_DECREASE,

	FLOAT_ASSIGN,
	FLOAT_ADD,
	FLOAT_ADD_ASSIGN,
	FLOAT_SUBTARCT,
	FLOAT_SUBTRACT_ASSIGN,
	FLOAT_MULTIPLY,
	FLOAT_MULTIPLY_ASSIGN,
	FLOAT_DIVIDE,
	FLOAT_DIVIDE_ASSIGN,
	FLOAT_EQUAL,
	FLOAT_NOT_EQUAL,
	FLOAT_GREATER,
	FLOAT_GREATER_EQUAL,
	FLOAT_SMALLER,
	FLOAT_SMALLER_EQUAL,
	FLOAT_NEGATIVE,

	FLOAT64_ASSIGN,
	FLOAT64_ADD,
	FLOAT64_ADD_ASSIGN,
	FLOAT64_SUBTRACT,
	FLOAT64_SUBTRACT_ASSIGN,
	FLOAT64_MULTIPLY,
	FLOAT64_MULTIPLY_ASSIGN,
	FLOAT64_DIVIDE,
	FLOAT64_DIVIDE_ASSIGN,
	FLOAT64_EQUAL,
	FLOAT64_NOT_EQUAL,
	FLOAT64_GREATER,
	FLOAT64_GREATER_EQUAL,
	FLOAT64_SMALLER,
	FLOAT64_SMALLER_EQUAL,
	FLOAT64_NEGATIVE,

//	COMPLEX_ASSIGN,
	VEC2_ADD,
	VEC2_ADD_ASSIGN,
	VEC2_SUBTRACT,
	VEC2_SUBTARCT_ASSIGN,
	VEC2_MULTIPLY_SV,
	VEC2_MULTIPLY_VS,
	VEC2_DIVIDE_VS,
	VEC2_NEGATIVE,

	COMPLEX_MULTIPLY,
	COMPLEX_MULTIPLY_ASSIGN,
	COMPLEX_DIVIDE,
	COMPLEX_DIVIDE_ASSIGN,

	CHUNK_ASSIGN,
	CHUNK_EQUAL,
	CHUNK_NOT_EQUAL,
	PASSTHROUGH,

	VECTOR_ADD,
	VECTOR_ADD_ASSIGN,
	VECTOR_SUBTRACT,
	VECTOR_SUBTARCT_ASSIGN,
	VECTOR_MULTIPLY_VV,
	VECTOR_MULTIPLY_VF,
	VECTOR_MULTIPLY_FV,
	VECTOR_MULTIPLY_ASSIGN,
	VECTOR_DIVIDE_VF,
	VECTOR_DIVIDE_ASSIGN,
	VECTOR_NEGATIVE,

	SHARED_POINTER_INIT,
};

}

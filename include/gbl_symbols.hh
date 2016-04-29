// Copyright (C) 2016 Gabriel Gouvine - All Rights Reserved

#if defined(GBL_DECL_ARRAY) || !defined(GBL_SYMBOLS_HH)

#ifndef GBL_DECL_ARRAY
#define GBL_SYMBOLS_HH
#endif

#ifndef GBL_DECL_ARRAY
// Expand into an enum

#include "private/gbl_forward_declarations.hh"

#ifdef __cplusplus
// C++ case: namespace allow us to use gbl::Symbol::*
#define GBL_BEGIN_DECL enum Symbol {
#define GBL_END_DECL , ENUM_MAX_SYMBOL };
#define GBL_DECL_SYMBOL( NAME ) NAME
#define GBL_DECL_EMPTY ENUM_NULL_SYMBOL
#else
// C case: the symbols are names GBL_SYMBOL_*
#define GBL_BEGIN_DECL enum gbl_symbol {
#define GBL_END_DECL , ENUM_MAX_SYMBOL };
#define GBL_DECL_SYMBOL( NAME ) GBL_SYMBOL_##NAME
#define GBL_DECL_EMPTY ENUM_NULL_SYMBOL
#endif

#else
// Expands into array of strings "GBL_*" ifdef GBL_DECL_ARRAY

#define GBL_BEGIN_DECL char const * const symbolStrings[] = {
#define GBL_END_DECL };
#define GBL_DECL_SYMBOL( NAME ) "GBL_"#NAME
#define GBL_DECL_EMPTY NULL
#endif

#if defined( __cplusplus) && !defined(GBL_DECL_ARRAY)
namespace gbl {
namespace Symbol {
#endif

GBL_BEGIN_DECL
  GBL_DECL_EMPTY, // Occupies index 0, not translated
  GBL_DECL_SYMBOL(CONSTANT_ONE),
  GBL_DECL_SYMBOL(CONSTANT_ZERO),
  GBL_DECL_SYMBOL(DIR_IN),
  GBL_DECL_SYMBOL(DIR_INOUT),
  GBL_DECL_SYMBOL(DIR_OUT),
  GBL_DECL_SYMBOL(VCC),
  GBL_DECL_SYMBOL(VSS)
GBL_END_DECL

#if defined( __cplusplus) && !defined(GBL_DECL_ARRAY)
} // End namespace Symbol
} // End namespace gbl
#endif

#undef GBL_DECL_SYMBOL
#undef GBL_DECL_EMPTY
#undef GBL_BEGIN_DECL
#undef GBL_END_DECL

#endif


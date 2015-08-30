/*
// ts_mruby_core.h - ts_mruby mruby module header
//
*/

#ifndef TS_MRUBY_CORE_H
#define TS_MRUBY_CORE_H

#include <mruby.h>

void ts_mrb_core_class_init(mrb_state *mrb, struct RClass *rclass);

#endif // TS_MRUBY_CORE_H

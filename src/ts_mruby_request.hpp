/*
// ts_mruby_request.h - ts_mruby mruby module header
//
*/

#ifndef TS_MRUBY_REQUEST_H
#define TS_MRUBY_REQUEST_H

#include <mruby.h>

void ts_mrb_request_class_init(mrb_state *mrb, struct RClass *rclass);

#endif // TS_MRUBY_REQUEST_H

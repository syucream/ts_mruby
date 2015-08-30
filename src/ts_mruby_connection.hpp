/*
// ts_mruby_connection.h - ts_mruby mruby module header
//
*/

#ifndef TS_MRUBY_CONNECTION_H
#define TS_MRUBY_CONNECTION_H

#include <mruby.h>

void ts_mrb_connection_class_init(mrb_state *mrb, struct RClass *rclass);

#endif // TS_MRUBY_CONNECTION_H

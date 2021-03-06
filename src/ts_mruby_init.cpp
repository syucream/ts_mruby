/*
// ts_mruby_init.c - ts_mruby mruby init functions
//
*/

#include "ts_mruby_init.hpp"
#include "ts_mruby_connection.hpp"
#include "ts_mruby_core.hpp"
#include "ts_mruby_eventsystem.hpp"
#include "ts_mruby_filter.hpp"
#include "ts_mruby_records.hpp"
#include "ts_mruby_request.hpp"
#include "ts_mruby_upstream.hpp"

#include <mruby.h>
#include <mruby/compile.h>

void ts_mrb_class_init(mrb_state *mrb) {
  struct RClass *rclass;

  rclass = mrb_define_class(mrb, "ATS", mrb->object_class);

  ts_mrb_connection_class_init(mrb, rclass);
  ts_mrb_core_class_init(mrb, rclass);
  ts_mrb_filter_class_init(mrb, rclass);
  ts_mrb_request_class_init(mrb, rclass);
  ts_mrb_upstream_class_init(mrb, rclass);
  ts_mrb_records_class_init(mrb, rclass);
  ts_mrb_eventsystem_class_init(mrb, rclass);
}

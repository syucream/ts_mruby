/*
// ts_mruby_filter.c - ts_mruby mruby module
//
*/

#include "ts_mruby_filter.hpp"
#include "ts_mruby_internal.hpp"

#include <atscppapi/Transaction.h>
#include <atscppapi/utils.h>

#include <mruby.h>
#include <mruby/class.h>
#include <mruby/compile.h>
#include <mruby/data.h>
#include <mruby/hash.h>
#include <mruby/proc.h>
#include <mruby/string.h>
#include <mruby/variable.h>

using namespace atscppapi;
using std::string;

static FilterPlugin *get_filter_plugin(TSMrubyContext *context) {
  context->registerFilterPlugin();
  return context->getFilterPlugin();
}

static mrb_value ts_mrb_set_filter_body(mrb_state *mrb, mrb_value self) {
  mrb_value argv;
  mrb_get_args(mrb, "o", &argv);
  if (mrb_type(argv) != MRB_TT_STRING) {
    argv = mrb_funcall(mrb, argv, "to_s", 0, NULL);
  }
  const string msg(static_cast<char *>(RSTRING_PTR(argv)), RSTRING_LEN(argv));

  auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  get_filter_plugin(context)->appendBody(msg);

  return self;
}

static mrb_value ts_mrb_transform_filter_body_bang(mrb_state *mrb, mrb_value self) {
  // Enable to receive both of block and Proc obj
  mrb_int argc;
  mrb_value *argv, block;
  mrb_get_args(mrb, "*&", &argv, &argc, &block);
  if (argc && mrb_nil_p(block)) {
    block = argv[0];
  }

  if (!mrb_nil_p(block) && mrb_type(block) == MRB_TT_PROC) {
    auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
    get_filter_plugin(context)->appendBlock(block);
  }

  return self;
}

void ts_mrb_filter_class_init(mrb_state *mrb, struct RClass *rclass) {
  struct RClass *class_filter;

  // Filter::
  class_filter =
      mrb_define_class_under(mrb, rclass, "Filter", mrb->object_class);
  mrb_define_method(mrb, class_filter, "body=", ts_mrb_set_filter_body,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, class_filter, "output=", ts_mrb_set_filter_body,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, class_filter, "transform!", ts_mrb_transform_filter_body_bang,
                    MRB_ARGS_REQ(1));
}

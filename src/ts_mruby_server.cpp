/*
// ts_mruby_server.c - ts_mruby mruby module
//
*/

#include "ts_mruby_server.hpp"

#include <mruby.h>
#include <mruby/proc.h>
#include <mruby/data.h>
#include <mruby/compile.h>
#include <mruby/string.h>
#include <mruby/class.h>

void ts_mrb_server_class_init(mrb_state *mrb, struct RClass *rclass)
{
  struct RClass *class_server;

  class_server = mrb_define_class_under(mrb, rclass, "Server", mrb->object_class);
}

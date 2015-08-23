/*
// ts_mruby_request.c - ts_mruby mruby module
//
*/

#include "ts_mruby_request.hpp"

#include <mruby.h>
#include <mruby/proc.h>
#include <mruby/data.h>
#include <mruby/compile.h>
#include <mruby/string.h>
#include <mruby/class.h>

using namespace atscppapi;

Transaction *ts_mruby_transaction;

void ts_mrb_set_transaction(Transaction *t)
{
  ts_mruby_transaction = t;
}

Transaction *ts_mrb_get_transaction(void)
{
  return ts_mruby_transaction;
}


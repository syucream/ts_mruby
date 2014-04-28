/*
// ts_mruby_request.h - ts_mruby mruby module header
//
*/

#ifndef TS_MRUBY_REQUEST_H
#define TS_MRUBY_REQUEST_H

#include <atscppapi/Transaction.h>

#include <mruby.h>
#include <mruby/hash.h>
#include <mruby/variable.h>

void ts_mrb_set_transaction(atscppapi::Transaction *t);
atscppapi::Transaction *ts_mrb_get_transaction(void);

#endif // TS_MRUBY_REQUEST_H

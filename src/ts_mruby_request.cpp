/*
// ts_mruby_request.c - ts_mruby mruby module
//
*/

#include "ts_mruby_request.hpp"
#include "ts_mruby_internal.hpp"

#include <atscppapi/Transaction.h>

#include <mruby.h>
#include <mruby/proc.h>
#include <mruby/data.h>
#include <mruby/compile.h>
#include <mruby/hash.h>
#include <mruby/string.h>
#include <mruby/class.h>
#include <mruby/variable.h>

using namespace atscppapi;
using std::string;

static mrb_value ts_mrb_get_class_obj(mrb_state *mrb, mrb_value self,
                                      char *obj_id, char *class_name)
{
  mrb_value obj;
  struct RClass *obj_class, *ts_class;

  obj = mrb_iv_get(mrb, self, mrb_intern_cstr(mrb, obj_id));
  if (mrb_nil_p(obj)) {
    ts_class = mrb_class_get(mrb, "ATS");
    obj_class = (struct RClass *)mrb_class_ptr(mrb_const_get(
        mrb, mrb_obj_value(ts_class), mrb_intern_cstr(mrb, class_name)));
    obj = mrb_obj_new(mrb, obj_class, 0, NULL);
    mrb_iv_set(mrb, self, mrb_intern_cstr(mrb, obj_id), obj);
  }
  return obj;
}

static mrb_value ts_mrb_get_request_header(mrb_state *mrb, Headers& headers)
{
  char *mkey;
  mrb_int mlen;
  mrb_get_args(mrb, "s", &mkey, &mlen);
  const string key(mkey, mlen);

  string headerField = headers[key].str();
  if (headerField != "") {
    return mrb_str_new(mrb, headerField.c_str(), headerField.length());
  } else {
    return mrb_nil_value();
  }
}

static mrb_value ts_mrb_headers_in_obj(mrb_state *mrb, mrb_value self)
{
  return ts_mrb_get_class_obj(mrb, self, (char*)"headers_in_obj", (char*)"Headers_in");
}

static mrb_value ts_mrb_get_request_headers_in(mrb_state *mrb, mrb_value self)
{
  TSMrubyContext* context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction* transaction = context->transaction;

  Headers& headers = transaction->getClientRequest().getHeaders();
  return ts_mrb_get_request_header(mrb, headers);
}

static mrb_value ts_mrb_set_request_headers_in(mrb_state *mrb, mrb_value self)
{
  mrb_value key, val;
  mrb_get_args(mrb, "oo", &key, &val);

  string key_str(RSTRING_PTR(key), RSTRING_LEN(key));
  string val_str(RSTRING_PTR(val), RSTRING_LEN(val));

  TSMrubyContext* context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction* transaction = context->transaction;

  Headers& headers = transaction->getClientRequest().getHeaders();
  headers.set(key_str, val_str);

  return self;
}

void ts_mrb_request_class_init(mrb_state *mrb, struct RClass *rclass)
{
  struct RClass *class_request;
  struct RClass *class_headers_in;

  // Request::
  class_request =
    mrb_define_class_under(mrb, rclass, "Request", mrb->object_class);

  mrb_define_method(mrb, class_request, "headers_in", ts_mrb_headers_in_obj,
                    MRB_ARGS_NONE());

  // Request::headers_in
  class_headers_in =
    mrb_define_class_under(mrb, rclass, "Headers_in", mrb->object_class);

  mrb_define_method(mrb, class_headers_in, "[]", ts_mrb_get_request_headers_in,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, class_headers_in, "[]=",
                    ts_mrb_set_request_headers_in, MRB_ARGS_ANY());
}

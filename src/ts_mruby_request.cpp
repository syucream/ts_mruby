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

static mrb_value ts_mrb_get_request_uri(mrb_state *mrb, mrb_value self)
{
  const TSMrubyContext* context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction* transaction = context->transaction;

  const string path = "/" + transaction->getClientRequest().getUrl().getPath();
  return mrb_str_new(mrb, path.c_str(), path.length());
}

static mrb_value ts_mrb_set_request_uri(mrb_state *mrb, mrb_value self)
{
  char *muri;
  mrb_int mlen;
  mrb_get_args(mrb, "s", &muri, &mlen);
  const string uri(muri, mlen);

  TSMrubyContext* context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction* transaction = context->transaction;
  transaction->getClientRequest().getUrl().setPath(uri);

  return self;
}

static mrb_value ts_mrb_get_request_unparsed_uri(mrb_state *mrb, mrb_value self)
{
  const TSMrubyContext* context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction* transaction = context->transaction;

  const Url& url = transaction->getClientRequest().getUrl();
  string unparsedUri = "/" + url.getPath();
  unparsedUri += "?" + url.getQuery();

  return mrb_str_new(mrb, unparsedUri.c_str(), unparsedUri.length());
}

static mrb_value ts_mrb_set_request_unparsed_uri(mrb_state *mrb, mrb_value self)
{
  char *muri;
  mrb_int mlen;
  mrb_get_args(mrb, "s", &muri, &mlen);
  const string unparsedUri(muri, mlen);

  const TSMrubyContext* context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction* transaction = context->transaction;
  Url& url = transaction->getClientRequest().getUrl();

  size_t found = unparsedUri.find_first_of('?');
  if (found != string::npos) {
    url.setPath(string(unparsedUri.c_str(), found));

    if (unparsedUri.length() > found + 1) {
      url.setQuery(string(unparsedUri.c_str() + found + 1, unparsedUri.length() - found - 1));
    }
  } else {
    url.setPath(unparsedUri);
  }

  return self;
}

static mrb_value ts_mrb_get_request_method(mrb_state *mrb, mrb_value self)
{
  const TSMrubyContext* context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction* transaction = context->transaction;

  HttpMethod methodIndex = transaction->getClientRequest().getMethod();
  const string method = HTTP_METHOD_STRINGS[methodIndex];

  return mrb_str_new(mrb, method.c_str(), method.length());
}

static mrb_value ts_mrb_get_request_protocol(mrb_state *mrb, mrb_value self)
{
  const TSMrubyContext* context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction* transaction = context->transaction;

  // FIXME Currently SPDY, HTTP/2 and others are unsupported
  HttpVersion versionIndex = transaction->getClientRequest().getVersion();
  const string protocol = HTTP_VERSION_STRINGS[versionIndex];

  return mrb_str_new(mrb, protocol.c_str(), protocol.length());
}

static mrb_value ts_mrb_get_request_args(mrb_state *mrb, mrb_value self)
{
  const TSMrubyContext* context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction* transaction = context->transaction;

  const Url& url = transaction->getClientRequest().getUrl();
  const string args = url.getQuery();

  return mrb_str_new(mrb, args.c_str(), args.length());
}

static mrb_value ts_mrb_set_request_args(mrb_state *mrb, mrb_value self)
{
  char *margs;
  mrb_int mlen;
  mrb_get_args(mrb, "s", &margs, &mlen);
  const string args(margs, mlen);

  const TSMrubyContext* context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction* transaction = context->transaction;

  Url& url = transaction->getClientRequest().getUrl();
  url.setQuery(args);

  return self;
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

static mrb_value ts_mrb_del_request_headers_in(mrb_state *mrb, mrb_value self)
{
  char *mkey;
  mrb_int mlen;
  mrb_get_args(mrb, "s", &mkey, &mlen);
  const string key(mkey, mlen);

  TSMrubyContext* context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction* transaction = context->transaction;

  Headers& headers = transaction->getClientRequest().getHeaders();
  headers.erase(key);

  return self;
}

static mrb_value ts_mrb_get_request_headers_in_hash(mrb_state *mrb, mrb_value self)
{
  TSMrubyContext* context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction* transaction = context->transaction;

  Headers& headers = transaction->getClientRequest().getHeaders();
  mrb_value hash = mrb_hash_new(mrb);;

  const header_field_iterator end = headers.end();
  for (header_field_iterator it = headers.begin(); it != end; it++) {
    const string& headerName = (*it).name();
    const string& headerValue = (*it).values();

    mrb_value key = mrb_str_new(mrb, headerName.c_str(), headerName.length());
    mrb_value value = mrb_str_new(mrb, headerValue.c_str(), headerValue.length());
    mrb_hash_set(mrb, hash, key, value);
  }

  return hash;
}

void ts_mrb_request_class_init(mrb_state *mrb, struct RClass *rclass)
{
  struct RClass *class_request;
  struct RClass *class_headers_in;

  // Request::
  class_request =
    mrb_define_class_under(mrb, rclass, "Request", mrb->object_class);

  // XXX Unsupported: ATS doesn't support API's that access to request body
  // mrb_define_method(mrb, class_request, "get_body", ts_mrb_get_request_body,
  //                   MRB_ARGS_NONE());
  // mrb_define_method(mrb, class_request, "read_body", ts_mrb_read_request_body,
  //                   MRB_ARGS_NONE());

  // Unsupported yet
  // mrb_define_method(mrb, class_request, "content_type=",
  //                   ts_mrb_set_content_type, MRB_ARGS_ANY());
  // mrb_define_method(mrb, class_request, "content_type",
  //                   ts_mrb_get_content_type, MRB_ARGS_ANY());

  // XXX Unsupported: ATS doesn't support API's that overwrite request line
  // mrb_define_method(mrb, class_request, "request_line",
  //                   ts_mrb_get_request_request_line, MRB_ARGS_NONE());
  // mrb_define_method(mrb, class_request, "request_line=",
  //                   ts_mrb_set_request_request_line, MRB_ARGS_ANY());

  mrb_define_method(mrb, class_request, "uri", ts_mrb_get_request_uri,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, class_request, "uri=", ts_mrb_set_request_uri,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, class_request, "unparsed_uri",
                    ts_mrb_get_request_unparsed_uri, MRB_ARGS_NONE());
  mrb_define_method(mrb, class_request, "unparsed_uri=",
                    ts_mrb_set_request_unparsed_uri, MRB_ARGS_ANY());
  mrb_define_method(mrb, class_request, "method", ts_mrb_get_request_method,
                    MRB_ARGS_NONE());

  // XXX Unsupported: atscppapi doesn't support overwriting method
  // mrb_define_method(mrb, class_request, "method=", ts_mrb_set_request_method,
  //                   MRB_ARGS_ANY());

  mrb_define_method(mrb, class_request, "protocol",
                    ts_mrb_get_request_protocol, MRB_ARGS_NONE());

  // XXX Unsupported: atscppapi doesn't support overwriting version
  // mrb_define_method(mrb, class_request, "protocol=",
  //                   ngx_mrb_set_request_protocol, MRB_ARGS_ANY());

  mrb_define_method(mrb, class_request, "args", ts_mrb_get_request_args,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, class_request, "args=", ts_mrb_set_request_args,
                    MRB_ARGS_ANY());

  // Unsupported yet
  // mrb_define_method(mrb, class_request, "var", ts_mrb_get_request_var,
  //                   MRB_ARGS_NONE());

  mrb_define_method(mrb, class_request, "headers_in", ts_mrb_headers_in_obj,
                    MRB_ARGS_NONE());

  // Request::headers_in
  class_headers_in =
    mrb_define_class_under(mrb, rclass, "Headers_in", mrb->object_class);

  mrb_define_method(mrb, class_headers_in, "[]", ts_mrb_get_request_headers_in,
                    MRB_ARGS_ANY());
  mrb_define_method(mrb, class_headers_in, "[]=",
                    ts_mrb_set_request_headers_in, MRB_ARGS_ANY());
  mrb_define_method(mrb, class_headers_in, "delete",
                    ts_mrb_del_request_headers_in, MRB_ARGS_ANY());
  mrb_define_method(mrb, class_headers_in, "all",
                    ts_mrb_get_request_headers_in_hash, MRB_ARGS_ANY());

  // Unsupported yet
  // Request::headers_out
  // class_headers_out =
  //   mrb_define_class_under(mrb, rclass, "Headers_out", mrb->object_class);
  //
  // mrb_define_method(mrb, class_headers_out, "[]", ts_mrb_get_request_headers_out,
  //                   MRB_ARGS_ANY());
  // mrb_define_method(mrb, class_headers_out, "[]=",
  //                   ts_mrb_set_request_headers_out, MRB_ARGS_ANY());
  // mrb_define_method(mrb, class_headers_out, "delete",
  //                   ts_mrb_del_request_headers_out, MRB_ARGS_ANY());
  // mrb_define_method(mrb, class_headers_out, "all",
  //                   ts_mrb_get_request_headers_out_hash, MRB_ARGS_ANY());
}

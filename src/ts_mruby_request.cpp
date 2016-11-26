/*
// ts_mruby_request.c - ts_mruby mruby module
//
*/

#include "ts_mruby_request.hpp"
#include "ts_mruby_internal.hpp"

#include <ts/ts.h>
#include <atscppapi/Transaction.h>

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

const string CONTENT_TYPE_KEY = "Content-Type";

namespace {

static mrb_value ts_mrb_get_class_obj(mrb_state *mrb, mrb_value self,
                                      char *obj_id, char *class_name) {
  mrb_value obj;
  struct RClass *obj_class, *ts_class;

  obj = mrb_iv_get(mrb, self, mrb_intern_cstr(mrb, obj_id));
  if (mrb_nil_p(obj)) {
    ts_class = mrb_class_get(mrb, "ATS");
    obj_class = reinterpret_cast<struct RClass *>(mrb_class_ptr(mrb_const_get(
        mrb, mrb_obj_value(ts_class), mrb_intern_cstr(mrb, class_name))));
    obj = mrb_obj_new(mrb, obj_class, 0, NULL);
    mrb_iv_set(mrb, self, mrb_intern_cstr(mrb, obj_id), obj);
  }
  return obj;
}

static mrb_value ts_mrb_get_request_header(mrb_state *mrb, Headers &headers) {
  char *mkey;
  mrb_int mlen;
  mrb_get_args(mrb, "s", &mkey, &mlen);
  const string key(mkey, mlen);

  const string &headerField = headers[key].values();
  if (headerField != "") {
    return mrb_str_new(mrb, headerField.c_str(), headerField.length());
  } else {
    return mrb_nil_value();
  }
}

} // anonymous namespace

static mrb_value ts_mrb_headers_in_obj(mrb_state *mrb, mrb_value self) {
  return ts_mrb_get_class_obj(mrb, self, (char *)"headers_in_obj",
                              (char *)"Headers_in");
}

static mrb_value ts_mrb_get_scheme(mrb_state *mrb, mrb_value self) {
  auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction *transaction = context->getTransaction();

  const string &scheme = transaction->getClientRequest().getUrl().getScheme();
  return mrb_str_new(mrb, scheme.c_str(), scheme.length());
}

static mrb_value ts_mrb_get_content_type(mrb_state *mrb, mrb_value self) {
  auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction *transaction = context->getTransaction();
  Headers &headers = transaction->getClientRequest().getHeaders();

  const string &contype = headers[CONTENT_TYPE_KEY].values();
  if (contype != "") {
    return mrb_str_new(mrb, contype.c_str(), contype.length());
  } else {
    return mrb_nil_value();
  }
}

static mrb_value ts_mrb_set_content_type(mrb_state *mrb, mrb_value self) {
  char *mcontype;
  mrb_int mlen;
  mrb_get_args(mrb, "s", &mcontype, &mlen);
  const string contype(mcontype, mlen);

  auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction *transaction = context->getTransaction();

  Headers &headers = transaction->getClientRequest().getHeaders();
  headers.set(CONTENT_TYPE_KEY, contype);

  return self;
}

static mrb_value ts_mrb_get_hostname(mrb_state *mrb, mrb_value self) {
  auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction *transaction = context->getTransaction();
  const Url &url = transaction->getClientRequest().getUrl();

  const string &hostname = url.getHost();
  return mrb_str_new(mrb, hostname.c_str(), hostname.length());
}

static mrb_value ts_mrb_get_request_url(mrb_state *mrb, mrb_value self) {
  auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction *transaction = context->getTransaction();

  const string& url = transaction->getClientRequest().getUrl().getUrlString();
  return mrb_str_new(mrb, url.c_str(), url.length());
}

static mrb_value ts_mrb_get_request_uri(mrb_state *mrb, mrb_value self) {
  auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction *transaction = context->getTransaction();

  const string path = "/" + transaction->getClientRequest().getUrl().getPath();
  return mrb_str_new(mrb, path.c_str(), path.length());
}

static mrb_value ts_mrb_set_request_uri(mrb_state *mrb, mrb_value self) {
  char *muri;
  mrb_int mlen;
  mrb_get_args(mrb, "s", &muri, &mlen);
  const string uri(muri, mlen);

  auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction *transaction = context->getTransaction();
  transaction->getClientRequest().getUrl().setPath(uri);

  return self;
}

static mrb_value ts_mrb_get_request_unparsed_uri(mrb_state *mrb,
                                                 mrb_value self) {
  TSMrubyContext *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction *transaction = context->getTransaction();

  const Url &url = transaction->getClientRequest().getUrl();
  const string unparsedUri = "/" + url.getPath() + "?" + url.getQuery();

  return mrb_str_new(mrb, unparsedUri.c_str(), unparsedUri.length());
}

static mrb_value ts_mrb_set_request_unparsed_uri(mrb_state *mrb,
                                                 mrb_value self) {
  char *muri;
  mrb_int mlen;
  mrb_get_args(mrb, "s", &muri, &mlen);
  const string unparsedUri(muri, mlen);

  auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction *transaction = context->getTransaction();
  Url &url = transaction->getClientRequest().getUrl();

  size_t found = unparsedUri.find_first_of('?');
  if (found != string::npos) {
    url.setPath(string(unparsedUri.c_str(), found));

    if (unparsedUri.length() > found + 1) {
      url.setQuery(string(unparsedUri.c_str() + found + 1,
                          unparsedUri.length() - found - 1));
    }
  } else {
    url.setPath(unparsedUri);
  }

  return self;
}

static mrb_value ts_mrb_get_request_method(mrb_state *mrb, mrb_value self) {
  auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction *transaction = context->getTransaction();

  HttpMethod methodIndex = transaction->getClientRequest().getMethod();
  const string &method = HTTP_METHOD_STRINGS[methodIndex];

  return mrb_str_new(mrb, method.c_str(), method.length());
}

static mrb_value ts_mrb_set_request_method(mrb_state *mrb, mrb_value self) {
  char *m_method;
  mrb_int mlen;
  mrb_get_args(mrb, "s", &m_method, &mlen);
  const string method(m_method, mlen);

  auto *ctx = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction *transaction = ctx->getTransaction();

  // NOTE: The CPPAPI doesn't have a setter for method, so alternatively use TS API.
  auto ts_txn = reinterpret_cast<TSHttpTxn>(transaction->getAtsHandle());
  TSMBuffer hdr_buf;
  TSMLoc hdr_loc;
  TSHttpTxnClientReqGet(ts_txn, &hdr_buf, &hdr_loc);
  TSHttpHdrMethodSet(hdr_buf, hdr_loc, method.c_str(), method.length());

  return self;
}

static mrb_value ts_mrb_get_request_protocol(mrb_state *mrb, mrb_value self) {
  auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction *transaction = context->getTransaction();

  // FIXME Currently SPDY, HTTP/2 and others are unsupported
  HttpVersion versionIndex = transaction->getClientRequest().getVersion();
  const string &protocol = HTTP_VERSION_STRINGS[versionIndex];

  return mrb_str_new(mrb, protocol.c_str(), protocol.length());
}

static mrb_value ts_mrb_get_request_args(mrb_state *mrb, mrb_value self) {
  auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction *transaction = context->getTransaction();

  const Url &url = transaction->getClientRequest().getUrl();
  const string &args = url.getQuery();

  return mrb_str_new(mrb, args.c_str(), args.length());
}

static mrb_value ts_mrb_set_request_args(mrb_state *mrb, mrb_value self) {
  char *margs;
  mrb_int mlen;
  mrb_get_args(mrb, "s", &margs, &mlen);
  const string args(margs, mlen);

  auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction *transaction = context->getTransaction();

  Url &url = transaction->getClientRequest().getUrl();
  url.setQuery(args);

  return self;
}

static mrb_value ts_mrb_get_request_headers_in(mrb_state *mrb, mrb_value self) {
  auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction *transaction = context->getTransaction();

  Headers &headers = transaction->getClientRequest().getHeaders();
  return ts_mrb_get_request_header(mrb, headers);
}

static mrb_value ts_mrb_set_request_headers_in(mrb_state *mrb, mrb_value self) {
  mrb_value key, val;
  mrb_get_args(mrb, "oo", &key, &val);

  const string key_str(RSTRING_PTR(key), RSTRING_LEN(key));
  const string val_str(RSTRING_PTR(val), RSTRING_LEN(val));

  auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction *transaction = context->getTransaction();

  Headers &headers = transaction->getClientRequest().getHeaders();
  headers.set(key_str, val_str);

  return self;
}

static mrb_value ts_mrb_del_request_headers_in(mrb_state *mrb, mrb_value self) {
  char *mkey;
  mrb_int mlen;
  mrb_get_args(mrb, "s", &mkey, &mlen);
  const string key(mkey, mlen);

  auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction *transaction = context->getTransaction();

  Headers &headers = transaction->getClientRequest().getHeaders();
  headers.erase(key);

  return self;
}

static mrb_value ts_mrb_get_request_headers_in_hash(mrb_state *mrb,
                                                    mrb_value self) {
  auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction *transaction = context->getTransaction();

  Headers &headers = transaction->getClientRequest().getHeaders();
  mrb_value hash = mrb_hash_new(mrb);

  header_field_iterator end = headers.end();
  for (header_field_iterator it = headers.begin(); it != end; it++) {
    const string &headerName = (*it).name();
    const string &headerValue = (*it).values();

    const mrb_value key =
        mrb_str_new(mrb, headerName.c_str(), headerName.length());
    const mrb_value value =
        mrb_str_new(mrb, headerValue.c_str(), headerValue.length());
    mrb_hash_set(mrb, hash, key, value);
  }

  return hash;
}

static mrb_value ts_mrb_get_request_headers_out(mrb_state *mrb, mrb_value self) {
  auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);

  const auto current = context->getStateTag();
  if (current != TransactionStateTag::READ_RESPONSE_HEADERS &&
      current != TransactionStateTag::SEND_RESPONSE_HEADERS) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "Invalid event usage");
    return self;
  }

  auto *transaction = context->getTransaction();
  auto &headers = (current == TransactionStateTag::READ_RESPONSE_HEADERS) ?
    transaction->getServerResponse().getHeaders(): 
    transaction->getClientResponse().getHeaders();

  return ts_mrb_get_request_header(mrb, headers);
}

static mrb_value ts_mrb_set_request_headers_out(mrb_state *mrb,
                                                mrb_value self) {
  mrb_value key, val;
  mrb_get_args(mrb, "oo", &key, &val);

  const string key_str(RSTRING_PTR(key), RSTRING_LEN(key));
  const string val_str(RSTRING_PTR(val), RSTRING_LEN(val));

  auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);

  const auto current = context->getStateTag();
  if (current == TransactionStateTag::SEND_RESPONSE_HEADERS) {
    auto *transaction = context->getTransaction();
    auto &headers = transaction->getClientResponse().getHeaders();
    headers.set(key_str, val_str);
  } else {
    context->registerHeaderRewritePlugin();
    context->getHeaderRewritePlugin()->addRewriteRule(
        key_str, val_str, HeaderRewritePlugin::Operator::ASSIGN);
  }

  return self;
}

static mrb_value ts_mrb_del_request_headers_out(mrb_state *mrb,
                                                mrb_value self) {
  char *mkey;
  mrb_int mlen;
  mrb_get_args(mrb, "s", &mkey, &mlen);
  const string key(mkey, mlen);

  auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);

  const auto current = context->getStateTag();
  if (current == TransactionStateTag::SEND_RESPONSE_HEADERS) {
    auto *transaction = context->getTransaction();
    auto &headers = transaction->getClientResponse().getHeaders();
    headers.erase(key);
  } else {
    context->registerHeaderRewritePlugin();
    context->getHeaderRewritePlugin()->addRewriteRule(
        key, "", HeaderRewritePlugin::Operator::DELETE);
  }

  return self;
}

static mrb_value ts_mrb_get_request_headers_out_hash(mrb_state *mrb,
                                                    mrb_value self) {
  auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);

  const auto current = context->getStateTag();
  if (current != TransactionStateTag::READ_RESPONSE_HEADERS &&
      current != TransactionStateTag::SEND_RESPONSE_HEADERS) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "Invalid event usage");
    return self;
  }

  auto *transaction = context->getTransaction();
  auto &headers = (current == TransactionStateTag::READ_RESPONSE_HEADERS) ?
    transaction->getServerResponse().getHeaders(): 
    transaction->getClientResponse().getHeaders();

  mrb_value hash = mrb_hash_new(mrb);
  const auto end = headers.end();
  for (auto it = headers.begin(); it != end; it++) {
    const string &headerName = (*it).name();
    const string &headerValue = (*it).values();

    const mrb_value key =
        mrb_str_new(mrb, headerName.c_str(), headerName.length());
    const mrb_value value =
        mrb_str_new(mrb, headerValue.c_str(), headerValue.length());
    mrb_hash_set(mrb, hash, key, value);
  }

  return hash;
}

void ts_mrb_request_class_init(mrb_state *mrb, struct RClass *rclass) {
  struct RClass *class_request;
  struct RClass *class_headers_in;
  struct RClass *class_headers_out;

  // Request::
  class_request =
      mrb_define_class_under(mrb, rclass, "Request", mrb->object_class);

  // XXX Unsupported: ATS doesn't support API's that access to request body
  // mrb_define_method(mrb, class_request, "get_body", ts_mrb_get_request_body,
  //                   MRB_ARGS_NONE());
  // mrb_define_method(mrb, class_request, "read_body",
  // ts_mrb_read_request_body,
  //                   MRB_ARGS_NONE());

  mrb_define_method(mrb, class_request, "scheme", ts_mrb_get_scheme,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, class_request, "content_type", ts_mrb_get_content_type,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, class_request, "content_type=",
                    ts_mrb_set_content_type, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, class_request, "hostname",
                    ts_mrb_get_hostname, MRB_ARGS_NONE());

  // XXX Unsupported: ATS doesn't support API's that overwrite request line
  // mrb_define_method(mrb, class_request, "request_line",
  //                   ts_mrb_get_request_request_line, MRB_ARGS_NONE());
  // mrb_define_method(mrb, class_request, "request_line=",
  //                   ts_mrb_set_request_request_line, MRB_ARGS_ANY());

  mrb_define_method(mrb, class_request, "url", ts_mrb_get_request_url,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, class_request, "uri", ts_mrb_get_request_uri,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, class_request, "uri=", ts_mrb_set_request_uri,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, class_request, "unparsed_uri",
                    ts_mrb_get_request_unparsed_uri, MRB_ARGS_NONE());
  mrb_define_method(mrb, class_request, "unparsed_uri=",
                    ts_mrb_set_request_unparsed_uri, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, class_request, "method", ts_mrb_get_request_method,
                    MRB_ARGS_NONE());

  mrb_define_method(mrb, class_request, "method=", ts_mrb_set_request_method,
                    MRB_ARGS_REQ(1));

  mrb_define_method(mrb, class_request, "protocol", ts_mrb_get_request_protocol,
                    MRB_ARGS_NONE());

  // XXX Unsupported: atscppapi doesn't support overwriting version
  // mrb_define_method(mrb, class_request, "protocol=",
  //                   ngx_mrb_set_request_protocol, MRB_ARGS_ANY());

  mrb_define_method(mrb, class_request, "args", ts_mrb_get_request_args,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, class_request, "args=", ts_mrb_set_request_args,
                    MRB_ARGS_REQ(1));

  // Unsupported yet
  // mrb_define_method(mrb, class_request, "var", ts_mrb_get_request_var,
  //                   MRB_ARGS_NONE());

  mrb_define_method(mrb, class_request, "headers_in", ts_mrb_headers_in_obj,
                    MRB_ARGS_NONE());

  // Request::Headers_in
  class_headers_in =
      mrb_define_class_under(mrb, rclass, "Headers_in", mrb->object_class);

  mrb_define_method(mrb, class_headers_in, "[]", ts_mrb_get_request_headers_in,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, class_headers_in, "[]=", ts_mrb_set_request_headers_in,
                    MRB_ARGS_REQ(2));
  mrb_define_method(mrb, class_headers_in, "delete",
                    ts_mrb_del_request_headers_in, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, class_headers_in, "all",
                    ts_mrb_get_request_headers_in_hash, MRB_ARGS_NONE());

  // Request::Headers_out
  class_headers_out =
      mrb_define_class_under(mrb, rclass, "Headers_out", mrb->object_class);
  mrb_define_method(mrb, class_headers_out, "[]", ts_mrb_get_request_headers_out,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, class_headers_out, "[]=",
                    ts_mrb_set_request_headers_out, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, class_headers_out, "delete",
                    ts_mrb_del_request_headers_out, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, class_headers_out, "all",
                    ts_mrb_get_request_headers_out_hash, MRB_ARGS_NONE());

}

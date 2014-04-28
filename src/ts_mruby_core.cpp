/*
// ts_mruby_core.cpp - ts_mruby mruby module
//
*/

#include <iostream>
#include <sstream>

#include <atscppapi/HttpStatus.h>
#include <atscppapi/Transaction.h>
#include <atscppapi/InterceptPlugin.h>

#include <mruby.h>
#include <mruby/proc.h>
#include <mruby/data.h>
#include <mruby/compile.h>
#include <mruby/string.h>
#include <mruby/array.h>
#include <mruby/variable.h>

#include "ts_mruby_core.hpp"
#include "ts_mruby_request.hpp"

using namespace atscppapi;
using std::string;

class RputsPlugin : public InterceptPlugin {
private:
  const string _message;

  template <typename T>
  string toString(T num){
    std::stringstream ss;
    ss << num;
    return ss.str();
  }

public:
  RputsPlugin(Transaction &transaction, const string& msg)
    : InterceptPlugin(transaction, InterceptPlugin::TRANSACTION_INTERCEPT), _message(msg) { }

  void consume(const string &data, InterceptPlugin::RequestDataType type) {}

  void handleInputComplete(){
    string response("HTTP/1.1 200 OK\r\n"
                    "Content-Length: " + toString(_message.size()) + "\r\n"
                    "\r\n");
    InterceptPlugin::produce(response);
    response = _message + "\r\n";
    InterceptPlugin::produce(response);
    InterceptPlugin::setOutputComplete();
  }

};

static mrb_value ts_mrb_get_ts_mruby_name(mrb_state *mrb, mrb_value self)
{   
  return mrb_str_new_lit(mrb, MODULE_NAME);
}

static mrb_value ts_mrb_get_ts_mruby_version(mrb_state *mrb, mrb_value self)
{   
  return mrb_str_new_lit(mrb, MODULE_VERSION);
}

static mrb_value ts_mrb_get_trafficserver_version(mrb_state *mrb, mrb_value self)
{
  return mrb_str_new_lit(mrb, "Apache Traffic Server 5.0.0");
}

static mrb_value ts_mrb_server_name(mrb_state *mrb, mrb_value self)
{
  return mrb_str_new_lit(mrb, "ApacheTrafficServer");
}

static mrb_value ts_mrb_rputs(mrb_state *mrb, mrb_value self)
{   
  mrb_value argv;
  mrb_get_args(mrb, "o", &argv);
  if (mrb_type(argv) != MRB_TT_STRING) {
    argv = mrb_funcall(mrb, argv, "to_s", 0, NULL);
  }
  const string msg((char*)RSTRING_PTR(argv), RSTRING_LEN(argv));

  atscppapi::Transaction* transaction = ts_mrb_get_transaction();
  transaction->addPlugin(new RputsPlugin(*transaction, msg));

  return self;
}

static mrb_value ts_mrb_echo(mrb_state *mrb, mrb_value self)
{   
  mrb_value argv;
  mrb_get_args(mrb, "o", &argv);
  if (mrb_type(argv) != MRB_TT_STRING) {
    argv = mrb_funcall(mrb, argv, "to_s", 0, NULL);
  }
  string msg((char*)RSTRING_PTR(argv), RSTRING_LEN(argv));
  msg += "\n";

  atscppapi::Transaction* transaction = ts_mrb_get_transaction();
  transaction->addPlugin(new RputsPlugin(*transaction, msg));

  return self;
}

void ts_mrb_core_class_init(mrb_state *mrb, struct RClass *rclass)
{
  mrb_define_method(mrb, mrb->kernel_module, "server_name", ts_mrb_server_name, ARGS_NONE());

  mrb_define_const(mrb, rclass, "HTTP_CONTINUE", mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_CONTINUE));
  mrb_define_const(mrb, rclass, "HTTP_SWITCHING_PROTOCOLS", mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_SWITCHING_PROTOCOL));
  mrb_define_const(mrb, rclass, "HTTP_OK",             mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_OK));
  mrb_define_const(mrb, rclass, "HTTP_CREATED",          mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_CREATED));
  mrb_define_const(mrb, rclass, "HTTP_ACCEPTED",           mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_ACCEPTED));
  mrb_define_const(mrb, rclass, "HTTP_NO_CONTENT",         mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_NO_CONTENT));
  mrb_define_const(mrb, rclass, "HTTP_PARTIAL_CONTENT",         mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_PARTIAL_CONTENT));
  mrb_define_const(mrb, rclass, "HTTP_MOVED_PERMANENTLY",      mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_MOVED_PERMANENTLY));
  mrb_define_const(mrb, rclass, "HTTP_MOVED_TEMPORARILY",      mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_MOVED_TEMPORARILY));
  mrb_define_const(mrb, rclass, "HTTP_SEE_OTHER",          mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_SEE_OTHER));
  mrb_define_const(mrb, rclass, "HTTP_NOT_MODIFIED",       mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_NOT_MODIFIED));
  mrb_define_const(mrb, rclass, "HTTP_TEMPORARY_REDIRECT", mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_TEMPORARY_REDIRECT));
  mrb_define_const(mrb, rclass, "HTTP_BAD_REQUEST",        mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_BAD_REQUEST));
  mrb_define_const(mrb, rclass, "HTTP_UNAUTHORIZED",       mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_UNAUTHORIZED));
  mrb_define_const(mrb, rclass, "HTTP_FORBIDDEN",          mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_FORBIDDEN));
  mrb_define_const(mrb, rclass, "HTTP_NOT_FOUND",          mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_NOT_FOUND));
  mrb_define_const(mrb, rclass, "HTTP_NOT_ALLOWED",        mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_METHOD_NOT_ALLOWED));
  mrb_define_const(mrb, rclass, "HTTP_REQUEST_TIME_OUT",   mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_REQUEST_TIMEOUT));
  mrb_define_const(mrb, rclass, "HTTP_CONFLICT",           mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_CONFLICT));
  mrb_define_const(mrb, rclass, "HTTP_LENGTH_REQUIRED",      mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_LENGTH_REQUIRED));
  mrb_define_const(mrb, rclass, "HTTP_PRECONDITION_FAILED",    mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_PRECONDITION_FAILED));
  mrb_define_const(mrb, rclass, "HTTP_REQUEST_ENTITY_TOO_LARGE",   mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_REQUEST_ENTITY_TOO_LARGE));
  mrb_define_const(mrb, rclass, "HTTP_REQUEST_URI_TOO_LARGE",    mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_REQUEST_URI_TOO_LONG));
  mrb_define_const(mrb, rclass, "HTTP_UNSUPPORTED_MEDIA_TYPE",   mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE));
  mrb_define_const(mrb, rclass, "HTTP_RANGE_NOT_SATISFIABLE",    mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_REQUESTED_RANGE_NOT_SATISFIABLE));
  mrb_define_const(mrb, rclass, "HTTP_REQUEST_HEADER_TOO_LARGE",   mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_REQUEST_HEADER_FIELDS_TOO_LARGE));
  mrb_define_const(mrb, rclass, "HTTP_INTERNAL_SERVER_ERROR",    mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_INTERNAL_SERVER_ERROR));
  mrb_define_const(mrb, rclass, "HTTP_NOT_IMPLEMENTED",      mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_NOT_IMPLEMENTED));
  mrb_define_const(mrb, rclass, "HTTP_BAD_GATEWAY",        mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_BAD_GATEWAY));
  mrb_define_const(mrb, rclass, "HTTP_SERVICE_UNAVAILABLE",    mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_SERVICE_UNAVAILABLE));
  mrb_define_const(mrb, rclass, "HTTP_GATEWAY_TIME_OUT",       mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_GATEWAY_TIMEOUT));
  mrb_define_const(mrb, rclass, "HTTP_INSUFFICIENT_STORAGE",     mrb_fixnum_value(atscppapi::HttpStatus::HTTP_STATUS_INSUFFICIENT_STORAGE));

  mrb_define_class_method(mrb, rclass, "rputs",           ts_mrb_rputs,           MRB_ARGS_ANY());
  mrb_define_class_method(mrb, rclass, "echo",            ts_mrb_echo,            MRB_ARGS_ANY());
  /* mrb_define_class_method(mrb, rclass, "return",             ts_mrb_send_header,        MRB_ARGS_ANY()); */
  mrb_define_class_method(mrb, rclass, "module_name",          ts_mrb_get_ts_mruby_name,     MRB_ARGS_NONE());
  mrb_define_class_method(mrb, rclass, "module_version",         ts_mrb_get_ts_mruby_version,    MRB_ARGS_NONE());
  mrb_define_class_method(mrb, rclass, "trafficserver_version",        ts_mrb_get_trafficserver_version,      MRB_ARGS_NONE());
  /* mrb_define_class_method(mrb, rclass, "redirect",           ts_mrb_redirect,           MRB_ARGS_ANY()); */
}

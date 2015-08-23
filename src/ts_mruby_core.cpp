/*
// ts_mruby_core.cpp - ts_mruby mruby module
//
*/

#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

#include <atscppapi/HttpStatus.h>
#include <atscppapi/Transaction.h>
#include <atscppapi/InterceptPlugin.h>
#include <atscppapi/Logger.h>

#include <mruby.h>
#include <mruby/proc.h>
#include <mruby/data.h>
#include <mruby/compile.h>
#include <mruby/string.h>
#include <mruby/array.h>
#include <mruby/variable.h>

#include "ts_mruby.hpp"
#include "ts_mruby_core.hpp"
#include "ts_mruby_request.hpp"

#include "ts_mruby_internal.hpp"

using namespace atscppapi;
using std::string;
using std::vector;
using std::pair;

namespace {

typedef vector<pair<string, string>> Headers;

class RputsPlugin : public InterceptPlugin {
private:
  int _statusCode;
  Headers _headers;
  string _message;

public:
  RputsPlugin(Transaction &transaction, int code=200)
    : InterceptPlugin(transaction, InterceptPlugin::TRANSACTION_INTERCEPT),
      _statusCode(code), _message("") { }

  ~RputsPlugin();

  void consume(const string &data, InterceptPlugin::RequestDataType type) {}

  void setStatusCode(int code) { _statusCode = code; }

  void appendMessage(const string msg) { _message += msg;  }

  void appendHeader(const pair<string, string> entry) { _headers.push_back(entry); }

  void appendHeaders(const Headers& h) {
    _headers.insert(_headers.end(), h.begin(), h.end());
  }

  void handleInputComplete(){
    string response("HTTP/1.1 " + 
                    std::to_string(_statusCode) + " " + 
                    reason_lookup(_statusCode) + "\r\n");

    // make response header
    if (!_message.empty()) {
      response += "Content-Length: " + std::to_string(_message.size()) + "\r\n";
    }

    for_each(_headers.begin(), _headers.end(),
             [&response](pair<string, string> entry) {
       response += entry.first + ": " + entry.second + "\r\n";
    });

    // make response body
    response += "\r\n";
    InterceptPlugin::produce(response);
    response = _message + "\r\n";
    InterceptPlugin::produce(response);
    InterceptPlugin::setOutputComplete();
  }

};

RputsPlugin *rputs = NULL;

RputsPlugin::~RputsPlugin() { rputs = NULL; }

} // namespace which has no name


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

  if (rputs == NULL) {
    atscppapi::Transaction* transaction = ts_mrb_get_transaction();
    rputs = new RputsPlugin(*transaction);
    transaction->addPlugin(rputs);
  }
  rputs->appendMessage(msg);

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

  if (rputs == NULL) {
    atscppapi::Transaction* transaction = ts_mrb_get_transaction();
    rputs = new RputsPlugin(*transaction);
    transaction->addPlugin(rputs);
  }
  rputs->appendMessage(msg);

  return self;
}

static mrb_value ts_mrb_send_header(mrb_state *mrb, mrb_value self)
{
  mrb_int statusCode;
  mrb_get_args(mrb, "i", &statusCode);

  if (rputs == NULL) {
    atscppapi::Transaction* transaction = ts_mrb_get_transaction();
    rputs = new RputsPlugin(*transaction, statusCode);
    transaction->addPlugin(rputs);
  } else {
    rputs->setStatusCode(statusCode);
  }

  return self;
}

static mrb_value ts_mrb_errlogger(mrb_state *mrb, mrb_value self)
{
  mrb_value *argv;
  mrb_value msg;
  mrb_int argc;
  mrb_int log_level;

  mrb_get_args(mrb, "*", &argv, &argc);

  if (argc != 2) {
    TS_ERROR(MODULE_NAME
      , "%s ERROR %s: argument is not 2"
      , MODULE_NAME
      , __func__
    );
    return self;
  }
  if (mrb_type(argv[0]) != MRB_TT_FIXNUM) {
    TS_ERROR(MODULE_NAME
      , "%s ERROR %s: argv[0] is not integer"
      , MODULE_NAME
      , __func__
    );
    return self;
  }
  log_level = mrb_fixnum(argv[0]);
  if (log_level < 0) {
    TS_ERROR(MODULE_NAME
      , "%s ERROR %s: log level is not positive number"
      , MODULE_NAME
      , __func__
    );
    return self;
  }

  if (mrb_type(argv[1]) != MRB_TT_STRING) {
    msg = mrb_funcall(mrb, argv[1], "to_s", 0, NULL);
  }
  else {
    msg = mrb_str_dup(mrb, argv[1]);
  }

  if (log_level == 0)
    TS_ERROR(MODULE_NAME, "%s", mrb_str_to_cstr(mrb, msg));
  else
    TS_DEBUG(MODULE_NAME, "%s", mrb_str_to_cstr(mrb, msg));

  return self;
}

static mrb_value ts_mrb_redirect(mrb_state *mrb, mrb_value self)
{
  int argc;
  mrb_value uri, code;
  int rc;

  argc = mrb_get_args(mrb, "o|oo", &uri, &code);

  // get status code from args
  if (argc == 2) {
    rc = mrb_fixnum(code);
  } else {
    rc = HttpStatus::HTTP_STATUS_MOVED_TEMPORARILY;
  }

  // get redirect uri from args
  if (mrb_type(uri) != MRB_TT_STRING) {
    uri = mrb_funcall(mrb, uri, "to_s", 0, NULL);
  }

  // save location uri
  const string redirectUri((char*)RSTRING_PTR(uri), RSTRING_LEN(uri));
  if (redirectUri.size() == 0) {
    return mrb_nil_value();
  }

  if (rputs == NULL) {
    Transaction* transaction = ts_mrb_get_transaction();
    rputs = new RputsPlugin(*transaction, HttpStatus::HTTP_STATUS_MOVED_TEMPORARILY);
    rputs->appendHeader(make_pair("Location", redirectUri));
    transaction->addPlugin(rputs);
  } else {
    rputs->setStatusCode(HttpStatus::HTTP_STATUS_MOVED_TEMPORARILY);
  }

  return self;
}

void ts_mrb_core_class_init(mrb_state *mrb, struct RClass *rclass)
{
  mrb_define_method(mrb, mrb->kernel_module, "server_name", ts_mrb_server_name, ARGS_NONE());

  // HTTP status codes
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

  // Log level
  mrb_define_const(mrb, rclass, "LOG_ERR",             mrb_fixnum_value(0));
  mrb_define_const(mrb, rclass, "LOG_DEBUG",             mrb_fixnum_value(1));

  mrb_define_class_method(mrb, rclass, "rputs",           ts_mrb_rputs,           MRB_ARGS_ANY());
  mrb_define_class_method(mrb, rclass, "echo",            ts_mrb_echo,            MRB_ARGS_ANY());
  mrb_define_class_method(mrb, rclass, "send_header",        ts_mrb_send_header,        MRB_ARGS_ANY());
  mrb_define_class_method(mrb, rclass, "return",             ts_mrb_send_header,        MRB_ARGS_ANY());
  mrb_define_class_method(mrb, rclass, "errlogger",          ts_mrb_errlogger,          MRB_ARGS_ANY());
  mrb_define_class_method(mrb, rclass, "module_name",          ts_mrb_get_ts_mruby_name,     MRB_ARGS_NONE());
  mrb_define_class_method(mrb, rclass, "module_version",         ts_mrb_get_ts_mruby_version,    MRB_ARGS_NONE());
  mrb_define_class_method(mrb, rclass, "trafficserver_version",        ts_mrb_get_trafficserver_version,      MRB_ARGS_NONE());
  mrb_define_class_method(mrb, rclass, "redirect",           ts_mrb_redirect,           MRB_ARGS_ANY());
}

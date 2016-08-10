/*
// ts_mruby_core.cpp - ts_mruby mruby module
//
*/

#include <algorithm>
#include <iostream>
#include <sstream>

#include <atscppapi/HttpStatus.h>
#include <atscppapi/Logger.h>
#include <atscppapi/Transaction.h>
#include <ts/ts.h>

#include <mruby.h>
#include <mruby/array.h>
#include <mruby/compile.h>
#include <mruby/data.h>
#include <mruby/proc.h>
#include <mruby/string.h>
#include <mruby/variable.h>

#include "ts_mruby_core.hpp"
#include "ts_mruby_internal.hpp"
#include "ts_mruby_request.hpp"

using namespace atscppapi;
using std::string;

static mrb_value ts_mrb_get_ts_mruby_name(mrb_state *mrb, mrb_value self) {
  return mrb_str_new_lit(mrb, TS_MRUBY_PLUGIN_NAME);
}

static mrb_value ts_mrb_get_ts_mruby_version(mrb_state *mrb, mrb_value self) {
  return mrb_str_new_cstr(mrb, TS_MRUBY_PLUGIN_VERSION);
}

static mrb_value ts_mrb_get_trafficserver_version(mrb_state *mrb,
                                                  mrb_value self) {
  const char *version = TSTrafficServerVersionGet();
  size_t len = strlen(version);

  return mrb_str_new(mrb, version, len);
}

static mrb_value ts_mrb_server_name(mrb_state *mrb, mrb_value self) {
  return mrb_str_new_lit(mrb, "ApacheTrafficServer");
}

static mrb_value ts_mrb_rputs(mrb_state *mrb, mrb_value self) {
  mrb_value argv;
  mrb_get_args(mrb, "o", &argv);
  if (mrb_type(argv) != MRB_TT_STRING) {
    argv = mrb_funcall(mrb, argv, "to_s", 0, NULL);
  }
  const string msg((char *)RSTRING_PTR(argv), RSTRING_LEN(argv));

  auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  if (context->rputs == NULL) {
    Transaction *transaction = context->transaction;

    context->rputs = new RputsPlugin(*transaction);
    transaction->addPlugin(context->rputs);
  }
  context->rputs->appendMessage(msg);

  return self;
}

static mrb_value ts_mrb_echo(mrb_state *mrb, mrb_value self) {
  mrb_value argv;
  mrb_get_args(mrb, "o", &argv);
  if (mrb_type(argv) != MRB_TT_STRING) {
    argv = mrb_funcall(mrb, argv, "to_s", 0, NULL);
  }
  string msg((char *)RSTRING_PTR(argv), RSTRING_LEN(argv));
  msg += "\n";

  auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  if (context->rputs == NULL) {
    Transaction *transaction = context->transaction;

    context->rputs = new RputsPlugin(*transaction);
    transaction->addPlugin(context->rputs);
  }
  context->rputs->appendMessage(msg);

  return self;
}

static mrb_value ts_mrb_send_header(mrb_state *mrb, mrb_value self) {
  mrb_int statusCode;
  mrb_get_args(mrb, "i", &statusCode);

  auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  if (context->rputs == NULL) {
    Transaction *transaction = context->transaction;

    context->rputs = new RputsPlugin(*transaction);
    transaction->addPlugin(context->rputs);
  } else {
    context->rputs->setStatusCode(statusCode);
  }

  return self;
}

static mrb_value ts_mrb_errlogger(mrb_state *mrb, mrb_value self) {
  mrb_value *argv;
  mrb_value msg;
  mrb_int argc;
  mrb_int log_level;

  mrb_get_args(mrb, "*", &argv, &argc);

  if (argc != 2) {
    TS_ERROR(TS_MRUBY_PLUGIN_NAME, "%s ERROR %s: argument is not 2", TS_MRUBY_PLUGIN_NAME,
             __func__);
    return self;
  }
  if (mrb_type(argv[0]) != MRB_TT_FIXNUM) {
    TS_ERROR(TS_MRUBY_PLUGIN_NAME, "%s ERROR %s: argv[0] is not integer", TS_MRUBY_PLUGIN_NAME,
             __func__);
    return self;
  }
  log_level = mrb_fixnum(argv[0]);
  if (log_level < 0) {
    TS_ERROR(TS_MRUBY_PLUGIN_NAME, "%s ERROR %s: log level is not positive number",
             TS_MRUBY_PLUGIN_NAME, __func__);
    return self;
  }

  if (mrb_type(argv[1]) != MRB_TT_STRING) {
    msg = mrb_funcall(mrb, argv[1], "to_s", 0, NULL);
  } else {
    msg = mrb_str_dup(mrb, argv[1]);
  }

  if (log_level == 0)
    TS_ERROR(TS_MRUBY_PLUGIN_NAME, "%s", mrb_str_to_cstr(mrb, msg));
  else
    TS_DEBUG(TS_MRUBY_PLUGIN_NAME, "%s", mrb_str_to_cstr(mrb, msg));

  return self;
}

static mrb_value ts_mrb_redirect(mrb_state *mrb, mrb_value self) {
  int argc;
  mrb_value uri, code;
  int status_code;

  argc = mrb_get_args(mrb, "o|oo", &uri, &code);

  // get status code from args
  if (argc == 2) {
    status_code = mrb_fixnum(code);
  } else {
    status_code = HttpStatus::HTTP_STATUS_MOVED_TEMPORARILY;
  }

  // get redirect uri from args
  if (mrb_type(uri) != MRB_TT_STRING) {
    uri = mrb_funcall(mrb, uri, "to_s", 0, NULL);
  }

  // save location uri
  const string redirectUri((char *)RSTRING_PTR(uri), RSTRING_LEN(uri));
  if (redirectUri.size() == 0) {
    return mrb_nil_value();
  }

  auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  if (context->rputs == NULL) {
    Transaction *transaction = context->transaction;

    context->rputs = new RputsPlugin(*transaction);
    context->rputs->appendHeader(make_pair("Location", redirectUri));
    transaction->addPlugin(context->rputs);
  } else {
    context->rputs->setStatusCode(status_code);
  }

  return self;
}

void ts_mrb_core_class_init(mrb_state *mrb, struct RClass *rclass) {
  mrb_define_method(mrb, mrb->kernel_module, "server_name", ts_mrb_server_name,
                    MRB_ARGS_NONE());

  // HTTP status codes
  mrb_define_const(mrb, rclass, "HTTP_CONTINUE",
                   mrb_fixnum_value(HttpStatus::HTTP_STATUS_CONTINUE));
  mrb_define_const(
      mrb, rclass, "HTTP_SWITCHING_PROTOCOLS",
      mrb_fixnum_value(HttpStatus::HTTP_STATUS_SWITCHING_PROTOCOL));
  mrb_define_const(mrb, rclass, "HTTP_OK",
                   mrb_fixnum_value(HttpStatus::HTTP_STATUS_OK));
  mrb_define_const(mrb, rclass, "HTTP_CREATED",
                   mrb_fixnum_value(HttpStatus::HTTP_STATUS_CREATED));
  mrb_define_const(mrb, rclass, "HTTP_ACCEPTED",
                   mrb_fixnum_value(HttpStatus::HTTP_STATUS_ACCEPTED));
  mrb_define_const(mrb, rclass, "HTTP_NO_CONTENT",
                   mrb_fixnum_value(HttpStatus::HTTP_STATUS_NO_CONTENT));
  mrb_define_const(mrb, rclass, "HTTP_PARTIAL_CONTENT",
                   mrb_fixnum_value(HttpStatus::HTTP_STATUS_PARTIAL_CONTENT));
  mrb_define_const(mrb, rclass, "HTTP_MOVED_PERMANENTLY",
                   mrb_fixnum_value(HttpStatus::HTTP_STATUS_MOVED_PERMANENTLY));
  mrb_define_const(mrb, rclass, "HTTP_MOVED_TEMPORARILY",
                   mrb_fixnum_value(HttpStatus::HTTP_STATUS_MOVED_TEMPORARILY));
  mrb_define_const(mrb, rclass, "HTTP_SEE_OTHER",
                   mrb_fixnum_value(HttpStatus::HTTP_STATUS_SEE_OTHER));
  mrb_define_const(mrb, rclass, "HTTP_NOT_MODIFIED",
                   mrb_fixnum_value(HttpStatus::HTTP_STATUS_NOT_MODIFIED));
  mrb_define_const(
      mrb, rclass, "HTTP_TEMPORARY_REDIRECT",
      mrb_fixnum_value(HttpStatus::HTTP_STATUS_TEMPORARY_REDIRECT));
  mrb_define_const(mrb, rclass, "HTTP_BAD_REQUEST",
                   mrb_fixnum_value(HttpStatus::HTTP_STATUS_BAD_REQUEST));
  mrb_define_const(mrb, rclass, "HTTP_UNAUTHORIZED",
                   mrb_fixnum_value(HttpStatus::HTTP_STATUS_UNAUTHORIZED));
  mrb_define_const(mrb, rclass, "HTTP_FORBIDDEN",
                   mrb_fixnum_value(HttpStatus::HTTP_STATUS_FORBIDDEN));
  mrb_define_const(mrb, rclass, "HTTP_NOT_FOUND",
                   mrb_fixnum_value(HttpStatus::HTTP_STATUS_NOT_FOUND));
  mrb_define_const(
      mrb, rclass, "HTTP_NOT_ALLOWED",
      mrb_fixnum_value(HttpStatus::HTTP_STATUS_METHOD_NOT_ALLOWED));
  mrb_define_const(mrb, rclass, "HTTP_REQUEST_TIME_OUT",
                   mrb_fixnum_value(HttpStatus::HTTP_STATUS_REQUEST_TIMEOUT));
  mrb_define_const(mrb, rclass, "HTTP_CONFLICT",
                   mrb_fixnum_value(HttpStatus::HTTP_STATUS_CONFLICT));
  mrb_define_const(mrb, rclass, "HTTP_LENGTH_REQUIRED",
                   mrb_fixnum_value(HttpStatus::HTTP_STATUS_LENGTH_REQUIRED));
  mrb_define_const(
      mrb, rclass, "HTTP_PRECONDITION_FAILED",
      mrb_fixnum_value(HttpStatus::HTTP_STATUS_PRECONDITION_FAILED));
  mrb_define_const(
      mrb, rclass, "HTTP_REQUEST_ENTITY_TOO_LARGE",
      mrb_fixnum_value(HttpStatus::HTTP_STATUS_REQUEST_ENTITY_TOO_LARGE));
  mrb_define_const(
      mrb, rclass, "HTTP_REQUEST_URI_TOO_LARGE",
      mrb_fixnum_value(HttpStatus::HTTP_STATUS_REQUEST_URI_TOO_LONG));
  mrb_define_const(
      mrb, rclass, "HTTP_UNSUPPORTED_MEDIA_TYPE",
      mrb_fixnum_value(HttpStatus::HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE));
  mrb_define_const(
      mrb, rclass, "HTTP_RANGE_NOT_SATISFIABLE",
      mrb_fixnum_value(
          HttpStatus::HTTP_STATUS_REQUESTED_RANGE_NOT_SATISFIABLE));
  mrb_define_const(
      mrb, rclass, "HTTP_REQUEST_HEADER_TOO_LARGE",
      mrb_fixnum_value(
          HttpStatus::HTTP_STATUS_REQUEST_HEADER_FIELDS_TOO_LARGE));
  mrb_define_const(
      mrb, rclass, "HTTP_INTERNAL_SERVER_ERROR",
      mrb_fixnum_value(HttpStatus::HTTP_STATUS_INTERNAL_SERVER_ERROR));
  mrb_define_const(mrb, rclass, "HTTP_NOT_IMPLEMENTED",
                   mrb_fixnum_value(HttpStatus::HTTP_STATUS_NOT_IMPLEMENTED));
  mrb_define_const(mrb, rclass, "HTTP_BAD_GATEWAY",
                   mrb_fixnum_value(HttpStatus::HTTP_STATUS_BAD_GATEWAY));
  mrb_define_const(
      mrb, rclass, "HTTP_SERVICE_UNAVAILABLE",
      mrb_fixnum_value(HttpStatus::HTTP_STATUS_SERVICE_UNAVAILABLE));
  mrb_define_const(mrb, rclass, "HTTP_GATEWAY_TIME_OUT",
                   mrb_fixnum_value(HttpStatus::HTTP_STATUS_GATEWAY_TIMEOUT));
  mrb_define_const(
      mrb, rclass, "HTTP_INSUFFICIENT_STORAGE",
      mrb_fixnum_value(HttpStatus::HTTP_STATUS_INSUFFICIENT_STORAGE));

  // Log level
  mrb_define_const(mrb, rclass, "LOG_ERR", mrb_fixnum_value(0));
  mrb_define_const(mrb, rclass, "LOG_DEBUG", mrb_fixnum_value(1));

  mrb_define_class_method(mrb, rclass, "rputs", ts_mrb_rputs, MRB_ARGS_ANY());
  mrb_define_class_method(mrb, rclass, "echo", ts_mrb_echo, MRB_ARGS_ANY());
  mrb_define_class_method(mrb, rclass, "send_header", ts_mrb_send_header,
                          MRB_ARGS_ANY());
  mrb_define_class_method(mrb, rclass, "return", ts_mrb_send_header,
                          MRB_ARGS_ANY());
  mrb_define_class_method(mrb, rclass, "errlogger", ts_mrb_errlogger,
                          MRB_ARGS_ANY());
  mrb_define_class_method(mrb, rclass, "module_name", ts_mrb_get_ts_mruby_name,
                          MRB_ARGS_NONE());
  mrb_define_class_method(mrb, rclass, "module_version",
                          ts_mrb_get_ts_mruby_version, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, rclass, "trafficserver_version",
                          ts_mrb_get_trafficserver_version, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, rclass, "redirect", ts_mrb_redirect,
                          MRB_ARGS_ANY());
}

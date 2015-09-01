/*
// ts_mruby_connection.c - ts_mruby mruby module
//
*/

#include "ts_mruby_connection.hpp"
#include "ts_mruby_internal.hpp"

#include <atscppapi/Transaction.h>
#include <atscppapi/utils.h>

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

static mrb_value ts_mrb_get_conn_var_remote_addr(mrb_state *mrb,
                                                 mrb_value self) {
  TSMrubyContext *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction *transaction = context->transaction;

  const sockaddr *addr = transaction->getClientAddress();
  const string remote_addr = utils::getIpString(addr);

  return mrb_str_new(mrb, remote_addr.c_str(), remote_addr.length());
}

static mrb_value ts_mrb_get_conn_var_remote_port(mrb_state *mrb,
                                                 mrb_value self) {
  TSMrubyContext *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction *transaction = context->transaction;

  const sockaddr *addr = transaction->getClientAddress();
  uint16_t remote_port = utils::getPort(addr);
  const string port_str = std::to_string(remote_port);

  return mrb_str_new(mrb, port_str.c_str(), port_str.length());
}

static mrb_value ts_mrb_get_conn_var_server_addr(mrb_state *mrb,
                                                 mrb_value self) {
  TSMrubyContext *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction *transaction = context->transaction;

  const sockaddr *addr = transaction->getIncomingAddress();
  const string server_addr = utils::getIpString(addr);

  return mrb_str_new(mrb, server_addr.c_str(), server_addr.length());
}

static mrb_value ts_mrb_get_conn_var_server_port(mrb_state *mrb,
                                                 mrb_value self) {
  TSMrubyContext *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  Transaction *transaction = context->transaction;

  const sockaddr *addr = transaction->getIncomingAddress();
  uint16_t server_port = utils::getPort(addr);
  const string port_str = std::to_string(server_port);

  return mrb_str_new(mrb, port_str.c_str(), port_str.length());
}

void ts_mrb_connection_class_init(mrb_state *mrb, struct RClass *rclass) {
  struct RClass *class_conn;

  // Connection::
  class_conn =
      mrb_define_class_under(mrb, rclass, "Connection", mrb->object_class);
  mrb_define_method(mrb, class_conn, "remote_ip",
                    ts_mrb_get_conn_var_remote_addr, MRB_ARGS_NONE());
  mrb_define_method(mrb, class_conn, "remote_port",
                    ts_mrb_get_conn_var_remote_port, MRB_ARGS_NONE());
  mrb_define_method(mrb, class_conn, "local_ip",
                    ts_mrb_get_conn_var_server_addr, MRB_ARGS_NONE());
  mrb_define_method(mrb, class_conn, "local_port",
                    ts_mrb_get_conn_var_server_port, MRB_ARGS_NONE());
}

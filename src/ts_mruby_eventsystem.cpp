/*
// ts_mruby_eventsystem.cpp - ts_mruby mruby module
//
*/

#include "ts_mruby_eventsystem.hpp"
#include "ts_mruby_internal.hpp"

#include <atscppapi/Transaction.h>
#include <atscppapi/utils.h>

#include <mruby.h>
#include <mruby/class.h>
#include <mruby/compile.h>
#include <mruby/data.h>
#include <mruby/proc.h>
#include <mruby/variable.h>

using namespace std;
using namespace atscppapi;

// TODO Support some aliases ... ?
static const string SEND_REQUEST_HDR_HANDLER  = "on_send_request_hdr";
static const string READ_RESPONSE_HDR_HANDLER = "on_read_response_hdr";
static const string SEND_RESPONSE_HDR_HANDLER = "on_send_response_hdr";

void
EventSystemPlugin::handleSendRequestHeaders(Transaction& transaction) {
  callHandler_(transaction, SEND_REQUEST_HDR_HANDLER);

  transaction.resume();
}

void
EventSystemPlugin::handleReadResponseHeaders(Transaction& transaction) {
  callHandler_(transaction, READ_RESPONSE_HDR_HANDLER);

  transaction.resume();
}

void
EventSystemPlugin::handleSendResponseHeaders(Transaction& transaction) {
  callHandler_(transaction, SEND_RESPONSE_HDR_HANDLER);

  transaction.resume();
}

mrb_value
EventSystemPlugin::callHandler_(Transaction& transaction, const string& sym) {
  auto* tlmrb = ts_mruby::getThreadLocalMrubyStates();
  mrb_state* mrb = tlmrb->getMrb();

  // Set context
  auto context_ = shared_ptr<TSMrubyContext>(new TSMrubyContext());
  context_->setTransaction(&transaction);
  mrb->ud = reinterpret_cast<void *>(context_.get());

  // Run mruby script
  mrb_value rv = mrb_funcall(mrb, handler_obj_->getValue(), sym.c_str(), 0, nullptr);

  return rv;
}

static mrb_value ts_mrb_register_es(mrb_state *mrb,
                                    mrb_value self) {
  mrb_value argv;
  mrb_get_args(mrb, "C", &argv);

  if (mrb_type(argv) != MRB_TT_CLASS) {
    // TODO raise exception ?
    return self;
  }
  struct RClass* handler_class = mrb_class_ptr(argv);

  auto* context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  auto* transaction = context->getTransaction();
  transaction->addPlugin(new EventSystemPlugin(*transaction, mrb, handler_class));

  return self;
}

void ts_mrb_eventsystem_class_init(mrb_state *mrb, struct RClass *rclass) {
  struct RClass *class_es;

  // EventSystem::
  class_es =
      mrb_define_class_under(mrb, rclass, "EventSystem", mrb->object_class);

  mrb_define_method(mrb, class_es, "register", ts_mrb_register_es, MRB_ARGS_REQ(1));
}

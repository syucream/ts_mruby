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
#include <mruby/dump.h>
#include <mruby/proc.h>
#include <mruby/variable.h>

using namespace std;
using namespace atscppapi;

static const char* SEND_REQUEST_HDR_HANDLER  = "on_send_request_hdr";
static const char* READ_RESPONSE_HDR_HANDLER = "on_read_response_hdr";
static const char* SEND_RESPONSE_HDR_HANDLER = "on_send_response_hdr";

EventSystemPlugin::EventSystemPlugin(atscppapi::Transaction &transaction, mrb_state* mrb, struct RClass* rclass)
    : atscppapi::TransactionPlugin(transaction) {
  
  size_t binsize = 0;
  if (mrb_obj_respond_to(mrb, rclass, mrb_intern_cstr(mrb, SEND_REQUEST_HDR_HANDLER))) {
    registerHook(HOOK_SEND_REQUEST_HEADERS);
    mrb_sym sym = mrb_intern_cstr(mrb, SEND_REQUEST_HDR_HANDLER);
    RProc* rproc = mrb_method_search(mrb, rclass, sym);
    mrb_dump_irep(mrb, rproc->body.irep, DUMP_ENDIAN_NAT, &send_request_hdr_irep_, &binsize);
  }
  if (mrb_obj_respond_to(mrb, rclass, mrb_intern_cstr(mrb, READ_RESPONSE_HDR_HANDLER))) {
    registerHook(HOOK_READ_RESPONSE_HEADERS);
    mrb_sym sym = mrb_intern_cstr(mrb, READ_RESPONSE_HDR_HANDLER);
    RProc* rproc = mrb_method_search(mrb, rclass, sym);
    mrb_dump_irep(mrb, rproc->body.irep, DUMP_ENDIAN_NAT, &read_response_hdr_irep_, &binsize);
  }
  if (mrb_obj_respond_to(mrb, rclass, mrb_intern_cstr(mrb, SEND_RESPONSE_HDR_HANDLER))) {
    registerHook(HOOK_SEND_RESPONSE_HEADERS);
    mrb_sym sym = mrb_intern_cstr(mrb, SEND_RESPONSE_HDR_HANDLER);
    RProc* rproc = mrb_method_search(mrb, rclass, sym);
    mrb_dump_irep(mrb, rproc->body.irep, DUMP_ENDIAN_NAT, &send_response_hdr_irep_, &binsize);
  }
}

EventSystemPlugin::~EventSystemPlugin() {
  auto* tlmrb = ts_mruby::getThreadLocalMrubyStates();
  mrb_state* mrb = tlmrb->getMrb();

  if (send_request_hdr_irep_) {
    mrb_free(mrb, send_request_hdr_irep_);
    send_request_hdr_irep_ = nullptr;
  }
  if (read_response_hdr_irep_) {
    mrb_free(mrb, read_response_hdr_irep_);
    read_response_hdr_irep_ = nullptr;
  }
  if (send_response_hdr_irep_) {
    mrb_free(mrb, send_response_hdr_irep_);
    send_response_hdr_irep_ = nullptr;
  }
}

void
EventSystemPlugin::handleSendRequestHeaders(Transaction& transaction) {
  auto context = shared_ptr<TSMrubyContext>(new TSMrubyContext());
  context->setTransaction(&transaction);
  context->setStateTag(TransactionStateTag::SEND_REQUEST_HEADERS);

  callHandler_(move(context), send_request_hdr_irep_);

  transaction.resume();
}

void
EventSystemPlugin::handleReadResponseHeaders(Transaction& transaction) {
  auto context = shared_ptr<TSMrubyContext>(new TSMrubyContext());
  context->setTransaction(&transaction);
  context->setStateTag(TransactionStateTag::READ_RESPONSE_HEADERS);

  callHandler_(move(context), read_response_hdr_irep_);

  transaction.resume();
}

void
EventSystemPlugin::handleSendResponseHeaders(Transaction& transaction) {
  auto context = shared_ptr<TSMrubyContext>(new TSMrubyContext());
  context->setTransaction(&transaction);
  context->setStateTag(TransactionStateTag::SEND_RESPONSE_HEADERS);

  callHandler_(move(context), send_response_hdr_irep_);

  transaction.resume();
}

mrb_value
EventSystemPlugin::callHandler_(shared_ptr<TSMrubyContext> context, const uint8_t* handler_irep) {
  auto* tlmrb = ts_mruby::getThreadLocalMrubyStates();
  mrb_state* mrb = tlmrb->getMrb();
  mrb->ud = reinterpret_cast<void *>(context.get());

  RProc* closure = mrb_closure_new(mrb, mrb_read_irep(mrb, handler_irep));

  // NOTE Its boxing_no specific
  mrb_value proc_value;
  BOXNIX_SET_VALUE(proc_value, MRB_TT_PROC, value.p, (closure));

  return mrb_yield(mrb, proc_value, mrb_nil_value());
}

static mrb_value ts_mrb_register_es(mrb_state *mrb,
                                    mrb_value self) {
  mrb_value argv;
  mrb_get_args(mrb, "C", &argv);
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

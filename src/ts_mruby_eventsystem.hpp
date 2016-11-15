/*
// ts_mruby_eventsystem.h - ts_mruby mruby module header
//
*/

#ifndef TS_MRUBY_EVENTSYSTEM_H
#define TS_MRUBY_EVENTSYSTEM_H

#include "ts_mruby_init.hpp"
#include "ts_mruby_internal.hpp"
#include <atscppapi/Transaction.h>
#include <atscppapi/TransactionPlugin.h>
#include <mruby.h>

// TODO Support some aliases ... ?
static const char* SEND_REQUEST_HDR_HANDLER  = "on_send_request_hdr";
static const char* READ_RESPONSE_HDR_HANDLER = "on_read_response_hdr";
static const char* SEND_RESPONSE_HDR_HANDLER = "on_send_response_hdr";

void ts_mrb_eventsystem_class_init(mrb_state *mrb, struct RClass *rclass);

/**
 * Management plugin of EventHandling and ATS hook.
 *
 */
class EventSystemPlugin : public atscppapi::TransactionPlugin {
private:
  std::shared_ptr<LentMrbValue> handler_obj_;
  mrb_value callHandler_(std::shared_ptr<TSMrubyContext>, const std::string&);

public:
  EventSystemPlugin(atscppapi::Transaction &transaction, mrb_state* mrb, struct RClass* rclass)
    : atscppapi::TransactionPlugin(transaction) {
    mrb_value v = mrb_obj_new(mrb, rclass, 0, nullptr);

    if (mrb_respond_to(mrb, v, mrb_intern_cstr(mrb, SEND_REQUEST_HDR_HANDLER))) {
      registerHook(HOOK_SEND_REQUEST_HEADERS);
    }
    if (mrb_respond_to(mrb, v, mrb_intern_cstr(mrb, READ_RESPONSE_HDR_HANDLER))) {
      registerHook(HOOK_READ_RESPONSE_HEADERS);
    }
    if (mrb_respond_to(mrb, v, mrb_intern_cstr(mrb, SEND_RESPONSE_HDR_HANDLER))) {
      registerHook(HOOK_SEND_RESPONSE_HEADERS);
    }

    // Should its handled in outer of this class?
    auto* tlmrb = ts_mruby::getThreadLocalMrubyStates();
    handler_obj_ = tlmrb->getManager().lend_mrb_value(v);
  }

  void handleSendRequestHeaders(atscppapi::Transaction&);
  void handleReadResponseHeaders(atscppapi::Transaction&);
  void handleSendResponseHeaders(atscppapi::Transaction&);
};

#endif // TS_MRUBY_EVENTSYSTEM_H

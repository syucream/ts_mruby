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

void ts_mrb_eventsystem_class_init(mrb_state *mrb, struct RClass *rclass);

/**
 * Management plugin of EventHandling and ATS hook.
 *
 */
class EventSystemPlugin : public atscppapi::TransactionPlugin {
private:
  std::shared_ptr<TSMrubyValue> handler_obj_;
  mrb_value callHandler_(atscppapi::Transaction&, const std::string&);

public:
  EventSystemPlugin(atscppapi::Transaction &transaction, mrb_state* mrb, struct RClass* rclass)
    : atscppapi::TransactionPlugin(transaction) {
    // TODO High cost. It should switch hook resistration by any criteria.
    registerHook(HOOK_SEND_REQUEST_HEADERS);
    registerHook(HOOK_READ_RESPONSE_HEADERS);
    registerHook(HOOK_SEND_RESPONSE_HEADERS);

    // Should its handled in outer of this class?
    mrb_value v = mrb_obj_new(mrb, rclass, 0, nullptr);

    handler_obj_ = std::shared_ptr<TSMrubyValue>(new TSMrubyValue(mrb, std::move(v)));
  }

  void handleSendRequestHeaders(atscppapi::Transaction&);
  void handleReadResponseHeaders(atscppapi::Transaction&);
  void handleSendResponseHeaders(atscppapi::Transaction&);
};

#endif // TS_MRUBY_EVENTSYSTEM_H

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
  uint8_t* send_request_hdr_irep_ = nullptr;
  uint8_t* read_response_hdr_irep_ = nullptr;
  uint8_t* send_response_hdr_irep_ = nullptr;

  mrb_value callHandler_(std::shared_ptr<TSMrubyContext>, const uint8_t*);

public:
  EventSystemPlugin(atscppapi::Transaction&, mrb_state*, struct RClass*);
  ~EventSystemPlugin();

  void handleSendRequestHeaders(atscppapi::Transaction&);
  void handleReadResponseHeaders(atscppapi::Transaction&);
  void handleSendResponseHeaders(atscppapi::Transaction&);
};

#endif // TS_MRUBY_EVENTSYSTEM_H

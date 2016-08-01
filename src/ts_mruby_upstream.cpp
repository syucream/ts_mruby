/*
// ts_mruby_upstream.cpp - ts_mruby mruby module
//
*/

#include "ts_mruby_upstream.hpp"
#include "ts_mruby_internal.hpp"

#include <atscppapi/Transaction.h>
#include <atscppapi/utils.h>

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

static mrb_value ts_mrb_set_server(mrb_state *mrb, mrb_value self) {
  char *mserver;
  mrb_int mlen;
  mrb_get_args(mrb, "s", &mserver, &mlen);
  const string server(mserver, mlen);

  auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  auto *transaction = context->transaction;

  Url& url = transaction->getClientRequest().getUrl();
  const auto authority = get_authority_pair(server, judge_tls(url.getScheme()));
  url.setHost(authority.first);
  url.setPort(authority.second);
  context->result.isRemapped = true;

  return self;
}

void ts_mrb_upstream_class_init(mrb_state *mrb, struct RClass *rclass) {
  struct RClass *class_upstream;

  // Upstream::
  class_upstream =
      mrb_define_class_under(mrb, rclass, "Upstream", mrb->object_class);
  mrb_define_method(mrb, class_upstream, "server=", ts_mrb_set_server,
                    MRB_ARGS_REQ(1));
}

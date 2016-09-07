#include <iostream>
#include <map>
#include <pthread.h>
#include <string>

#include <atscppapi/GlobalPlugin.h>
#include <atscppapi/PluginInit.h>
#include <atscppapi/RemapPlugin.h>

#include "ts_mruby_internal.hpp"

using namespace std;
using namespace atscppapi;

class MRubyPluginBase {
protected:
  MRubyPluginBase(const string &fpath) : filepath_(fpath) {}

  TSMrubyResult executeMrubyScript(Transaction &transaction) {
    // get or initialize thread local mruby VM
    auto* states = ts_mruby::getThreadLocalMrubyStates();
    mrb_state *mrb = states->getMrb();

    // get or compile mruby script
    RProc *proc = states->getRProc(filepath_);

    // set execution context
    context_ = shared_ptr<TSMrubyContext>(new TSMrubyContext());
    context_->setTransaction(&transaction);
    mrb->ud = reinterpret_cast<void *>(context_.get());

    // execute mruby script when ATS pre-remap hook occurs.
    mrb_run(mrb, proc, mrb_nil_value());

    return context_->getResult();
  }

private:
  string filepath_;
  shared_ptr<TSMrubyContext> context_;
};

class MRubyPlugin : public GlobalPlugin, MRubyPluginBase {
public:
  MRubyPlugin(const string &fpath) : MRubyPluginBase(fpath) {
    registerHook(HOOK_READ_REQUEST_HEADERS);
  }

  virtual void handleReadRequestHeaders(Transaction &transaction) {
    TSMrubyResult result = executeMrubyScript(transaction);

    if (result.isRemapped) {
      transaction.setSkipRemapping(1);
    }
    transaction.resume();
  }
};

class MRubyRemapPlugin : public RemapPlugin, MRubyPluginBase {
public:
  MRubyRemapPlugin(void **instance_handle, const string &fpath)
      : RemapPlugin(instance_handle), MRubyPluginBase(fpath) {}

  Result doRemap(const Url &map_from_url, const Url &map_to_url,
                 Transaction &transaction, bool &redirect) {
    TSMrubyResult result = executeMrubyScript(transaction);

    return (result.isRemapped) ? RESULT_DID_REMAP : RESULT_NO_REMAP;
  }
};

// As global plugin
void TSPluginInit(int argc, const char *argv[]) {
  if (argc == 2) {
    RegisterGlobalPlugin(TS_MRUBY_PLUGIN_NAME, TS_MRUBY_PLUGIN_AUTHOR,
                         TS_MRUBY_PLUGIN_EMAIL);

    ts_mruby::getInitializedGlobalScriptCache(argv[1]);
    new MRubyPlugin(argv[1]);
  }
}

// As remap plugin
TSReturnCode TSRemapNewInstance(int argc, char *argv[], void **ih,
                                char * /* ATS_UNUSED */, int /* ATS_UNUSED */) {
  if (argc == 3) {
    ts_mruby::getInitializedGlobalScriptCache(argv[2]);
    new MRubyRemapPlugin(ih, argv[2]);

    return TS_SUCCESS;
  } else {
    return TS_ERROR;
  }
}
